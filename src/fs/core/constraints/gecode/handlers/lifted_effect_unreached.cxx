	
#include <fs/core/languages/fstrips/language.hxx>
#include <fs/core/problem_info.hxx>
#include <fs/core/actions/actions.hxx>
#include <fs/core/actions/grounding.hxx>
#include <fs/core/constraints/gecode/handlers/lifted_effect_unreached.hxx>
#include <fs/core/constraints/gecode/handlers/formula_csp.hxx>
#include <fs/core/constraints/gecode/utils/novelty_constraints.hxx>
#include <fs/core/constraints/gecode/supports.hxx>
#include <fs/core/utils//printers/actions.hxx>
#include <lapkt/tools/logging.hxx>
#include <fs/core/heuristics/relaxed_plan/rpg_index.hxx>
#include <gecode/search.hh>

namespace fs0 { namespace gecode {


std::vector<std::unique_ptr<LiftedEffectUnreachedCSP>>
LiftedEffectUnreachedCSP::create(const std::vector<const PartiallyGroundedAction*>& schemata, const AtomIndex& tuple_index, bool approximate, bool novelty) {
	std::vector<std::unique_ptr<LiftedEffectUnreachedCSP>> handlers;
	
	for (auto schema:schemata) {
		assert(!schema->has_empty_parameter());
		
		for (auto eff:extract_non_delete_effects(*schema)) {
			// When creating an action CSP handler, it doesn't really make much sense to use the effect conditions.
			auto handler = std::unique_ptr<LiftedEffectUnreachedCSP>(new LiftedEffectUnreachedCSP(*schema, eff, tuple_index, approximate));
			
			if (!handler->init(novelty)) {
				LPT_DEBUG("grounding", "Action schema \"" << *schema << "\" detected as non-applicable before grounding");
				continue;
			}
			
			LPT_DEBUG("grounding", "Generated CSP for action schema " << *schema << std::endl <<  *handler << std::endl);
			handlers.push_back(std::move(handler));
		}
	}
	return handlers;
}


LiftedEffectUnreachedCSP::LiftedEffectUnreachedCSP(const PartiallyGroundedAction& action, const fs::ActionEffect* effect, const AtomIndex& tuple_index, bool approximate) :
	LiftedActionCSP(action, {effect}, tuple_index, approximate, true)
{}


bool
LiftedEffectUnreachedCSP::init(bool use_novelty_constraint) {
	if (!LiftedActionCSP::init(use_novelty_constraint)) return false;
	_lhs_subterm_variables = index_lhs_subterms();

	// Register all fluent symbols involved
	_tuple_indexes = _translator.index_fluents(_all_terms);
	
	return true;
}

void
LiftedEffectUnreachedCSP::log() const {
	assert(_effects.size() == 1);
	LPT_EDEBUG("heuristic", "Processing effect schema \"" << *get_effect() << " of action " << _action);
}

const fs::ActionEffect*
LiftedEffectUnreachedCSP::get_effect() const { 
	assert(_effects.size() == 1);
	return _effects[0];
}

void
LiftedEffectUnreachedCSP::create_novelty_constraint() {
	_novelty = NoveltyConstraint::createFromEffects(_translator, get_precondition(), get_effects());
}

void
LiftedEffectUnreachedCSP::post_novelty_constraint(GecodeCSP& csp, const RPGIndex& rpg) const {
	if (_novelty) _novelty->post_constraint(csp, rpg);
}

const fs::Formula*
LiftedEffectUnreachedCSP::get_precondition() const {
	return _action.getPrecondition();
}




bool LiftedEffectUnreachedCSP::find_atom_support(AtomIdx tuple, const Atom& atom, const State& seed, GecodeCSP& layer_csp, RPGIndex& rpg) const {
	log();
	
	std::unique_ptr<GecodeCSP> csp = std::unique_ptr<GecodeCSP>(static_cast<GecodeCSP*>(layer_csp.clone()));
	
	post_atom(*csp, atom);

	if (!csp->checkConsistency()) { // This colaterally enforces propagation of constraints
		LPT_EDEBUG("heuristic", "Action CSP inconsistent => atom " << atom << " cannot be derived through it");
		return false;
	} 
	
	// Else, the CSP is locally consistent
	if (_approximate) {  // Check only local consistency
		WORK_IN_PROGRESS("To be implemented");
		return true;
	} else { // Else, we want a full solution of the CSP
		return solve_for_tuple(tuple, csp.get(), rpg);
	}
}

bool LiftedEffectUnreachedCSP::solve_for_tuple(AtomIdx tuple, gecode::GecodeCSP* csp, RPGIndex& graph) const {
	// We just want to search for one solution and extract the support from it
	Gecode::DFS<GecodeCSP> engine(csp);
	GecodeCSP* solution = engine.next();
	if (!solution) return false; // The CSP has no solution at all
	
	bool reached = graph.reached(tuple);
	LPT_EDEBUG("heuristic", "Processing effect \"" << *get_effect() << "\" produces " << (reached ? "repeated" : "new") << " tuple " << tuple);
	
	if (reached) return true; // The value has already been reached before
	
	// Otherwise, the value is actually new - we extract the actual support from the solution
	std::vector<AtomIdx> support = Supports::extract_support(solution, _translator, _tuple_indexes, _necessary_tuples);
	graph.add(tuple, get_action_id(solution), std::move(support));

	delete solution;
	return true;
}


void LiftedEffectUnreachedCSP::post_atom(GecodeCSP& csp, const Atom& atom) const {
	const ProblemInfo& info = ProblemInfo::getInstance();
	const auto& effect = get_effect();
	if (auto statevar = dynamic_cast<const fs::StateVariable*>(effect->lhs())) {
		_unused(statevar);
		assert(statevar->getValue() == atom.getVariable()); // Otherwise we shouldn't be considering this effect as a potential achiever of atom.
	} else if (auto nested = dynamic_cast<const fs::FluentHeadedNestedTerm*>(effect->lhs())) {
		_unused(nested);
		const auto& data = info.getVariableData(atom.getVariable());
		assert(nested->getSymbolId() == data.first);
		assert(_lhs_subterm_variables.size() == data.second.size());
		for (unsigned i = 0; i < _lhs_subterm_variables.size(); ++i) {
			auto subterm_variable = _translator.resolveVariableFromIndex(_lhs_subterm_variables[i], csp);
			Gecode::rel(csp, subterm_variable,  Gecode::IRT_EQ, fs0::value<int>(data.second[i]));
		}
		
	} else throw std::runtime_error("Unknown effect type");
	
	// This is equivalent, but faster, to _translator.resolveVariable(effect->rhs(), csp);
	assert(effect_rhs_variables.size()==1);
	auto& rhs_term =_translator.resolveVariableFromIndex(effect_rhs_variables[0], csp);
	Gecode::rel(csp, rhs_term,  Gecode::IRT_EQ, fs0::value<int>(atom.getValue()));
}

std::vector<unsigned> LiftedEffectUnreachedCSP::index_lhs_subterms() {
	std::vector<unsigned> subterm_variables;
	auto lhs = get_effect()->lhs();
	if (auto nested = dynamic_cast<const fs::FluentHeadedNestedTerm*>(lhs)) {
		for (auto subterm:nested->getSubterms()) {
			subterm_variables.push_back(_translator.resolveVariableIndex(subterm));
		}
	}
	return subterm_variables;
}


GecodeCSP* LiftedEffectUnreachedCSP::preinstantiate(const RPGIndex& rpg) const {
	GecodeCSP* csp = instantiate(rpg);
	if (!csp) return nullptr;
	
	if (!csp->checkConsistency()) { // This colaterally enforces propagation of constraints
		delete csp;
		return nullptr;
	}
	
	return csp;
}





} } // namespaces

