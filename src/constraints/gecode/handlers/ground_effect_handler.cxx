
#include <languages/fstrips/language.hxx>
#include <constraints/gecode/handlers/ground_effect_handler.hxx>
#include <constraints/gecode/helper.hxx>
#include <constraints/gecode/supports.hxx>
#include <actions/actions.hxx>
#include <utils/printers/actions.hxx>
#include <aptk2/tools/logging.hxx>
#include <actions/action_id.hxx>
#include <problem.hxx>
#include <heuristics/relaxed_plan/rpg_index.hxx>
#include <state.hxx>
#include <gecode/driver.hh>

namespace fs0 { namespace gecode {

std::vector<std::shared_ptr<GroundEffectCSP>>
GroundEffectCSP::create(const std::vector<const GroundAction*>& actions, const TupleIndex& tuple_index, bool approximate, bool novelty) {
	std::vector<std::shared_ptr<GroundEffectCSP>> managers;
	
	for (unsigned action_idx = 0; action_idx < actions.size(); ++action_idx) {
		const auto action = actions[action_idx];
		
		for (unsigned eff_idx = 0; eff_idx < action->getEffects().size(); ++eff_idx) {
			const fs::ActionEffect* effect = action->getEffects().at(eff_idx);
			if (effect->is_del()) continue; // Ignore delete effects
			auto handler = std::make_shared<GroundEffectCSP>(*action, tuple_index, effect, approximate, true);
			if (handler->init(novelty)) {
				managers.push_back(handler);
				LPT_DEBUG("main", "Generated CSP for the effect #" << eff_idx << " of action " << print::action_header(*action) << std::endl <<  *handler << std::endl);
			} else {
				LPT_DEBUG("main", "CSP for action effect " << effect << " is inconsistent ==> the action is not applicable");
			}
		}
	}
	return managers;
}

GroundEffectCSP::GroundEffectCSP(const GroundAction& action, const TupleIndex& tuple_index, const fs::ActionEffect* effect, bool approximate, bool use_effect_conditions) :
	BaseActionCSP(tuple_index, approximate, use_effect_conditions), _action(action), _effects({ effect })
{}

bool GroundEffectCSP::init(bool use_novelty_constraint) {
	if (!BaseActionCSP::init(use_novelty_constraint)) return false;
	_lhs_subterm_variables = index_lhs_subterms();
	
	// Register all fluent symbols involved
	_tuple_indexes = _translator.index_fluents(_all_terms);
	
	return true;
}

void GroundEffectCSP::log() const {
	LPT_EDEBUG("heuristic", "Processing effect \"" << *get_effect() << "\" of action " << _action);
}

const ActionID* GroundEffectCSP::get_action_id(const GecodeCSP* solution) const {
	return new PlainActionID(&_action);
}

GecodeCSP* GroundEffectCSP::preinstantiate(const RPGIndex& rpg) const {
	GecodeCSP* csp = instantiate(rpg);
	if (!csp) return nullptr;
	
	if (!csp->checkConsistency()) { // This colaterally enforces propagation of constraints
		delete csp;
		return nullptr;
	}
	
	return csp;
}


bool GroundEffectCSP::find_atom_support(TupleIdx tuple, const Atom& atom, const State& seed, GecodeCSP& layer_csp, RPGIndex& rpg) const {
	log();
	
	std::unique_ptr<GecodeCSP> csp = std::unique_ptr<GecodeCSP>(static_cast<GecodeCSP*>(layer_csp.clone()));
	
	post(*csp, atom);

	if (!csp->checkConsistency()) { // This colaterally enforces propagation of constraints
		LPT_EDEBUG("heuristic", "Action CSP inconsistent => atom " << atom << " cannot be derived through it");
		return false;
	} 
	
	// Else, the CSP is locally consistent
	if (_approximate) {  // Check only local consistency
		WORK_IN_PROGRESS("To be implemented");
// 		solve_approximately(atom, csp.get(), rpg, seed);
		return true;
	} else { // Else, we want a full solution of the CSP
		return solve(tuple, csp.get(), rpg);
	}
}

bool GroundEffectCSP::solve(TupleIdx tuple, gecode::GecodeCSP* csp, RPGIndex& graph) const {
	// We just want to search for one solution an extract the support from it
	Gecode::DFS<GecodeCSP> engine(csp);
	GecodeCSP* solution = engine.next();
	if (!solution) return false; // The CSP has no solution at all
	
	bool reached = graph.reached(tuple);
	LPT_EDEBUG("heuristic", "Processing effect \"" << *get_effect() << "\" produces " << (reached ? "repeated" : "new") << " tuple " << tuple);
	
	if (reached) return true; // The value has already been reached before
	
	// Otherwise, the value is actually new - we extract the actual support from the solution
	std::vector<TupleIdx> support = Supports::extract_support(solution, _translator, _tuple_indexes, _necessary_tuples);
	graph.add(tuple, get_action_id(solution), std::move(support));

	delete solution;
	return true;
}



/*
void GroundEffectCSP::solve_approximately(const Atom& atom, gecode::GecodeCSP* csp, RPGData& rpg, const State& seed) const {
	// We have already propagated constraints with the call to status(), so we simply arbitrarily pick one consistent value per variable.
	
	Atom::vctrp support = std::make_shared<Atom::vctr>();

	// First process the direct state variables
	for (const auto& element:_translator.getAllInputVariables()) {
		VariableIdx variable = element.first;
		const Gecode::IntVar& csp_var = _translator.resolveVariableFromIndex(element.second, *csp);
		Gecode::IntVarValues values(csp_var);  // This returns a set with all consistent values for the given variable
		assert(values()); // Otherwise the CSP would be inconsistent!
		
		// If the original value makes the situation a goal, then we don't need to add anything for this variable.
		int seed_value = seed.getValue(variable);
		int selected = Helper::selectValueIfExists(values, seed_value);
		if (selected == seed_value) continue;
		support->push_back(Atom(variable, selected)); // It not, we simply pick the first consistent value
	}
	
	// Now the support of atoms such as 'clear(b)' that might appear in formulas in non-negated form.
	support->insert(support->end(), _atom_state_variables.begin(), _atom_state_variables.end());
	
	// TODO - This needs further thinking - ATM we simply ignore nested fluents, which will make the approximation approach work quite bad when the problem has them.
	const auto& nested_terms = effect_nested_fluents[0];
	if (0 && !nested_terms.empty()) {

		// And now of the derived state variables. Note that we keep track dynamically (with the 'insert' set) of the actual variables into which
		// the CSP solution resolves to prevent repetitions
		std::set<VariableIdx> inserted;

		
		const ProblemInfo& info = ProblemInfo::getInstance();
		
		for (fs::FluentHeadedNestedTerm::cptr fluent:nested_terms) {
			VariableIdx variable = info.resolveStateVariable(fluent->getSymbolId(), _translator.resolveValues(fluent->getSubterms(), *csp));
	//		VariableIdx variable = fluent->interpretVariable(assignment, binding);
			if (inserted.find(variable) == inserted.end()) { // Don't push twice the support the same atom
				// ObjectIdx value = fluent->interpret(assignment, binding);
				
				ObjectIdx value = 1; // i.e. assuming that there are no negated atoms on conditions.
				if (!info.isPredicate(fluent->getSymbolId())) {
					value = _translator.resolveValue(fluent, *csp);
				}
				
				support->push_back(Atom(variable, value));
				inserted.insert(variable);
			}
		}
	}
	
	rpg.add(atom, get_action_id(csp), support);
}
*/


void GroundEffectCSP::post(GecodeCSP& csp, const Atom& atom) const {
	const ProblemInfo& info = ProblemInfo::getInstance();
	assert(_effects.size() == 1);
	const auto& effect = _effects[0];
	if (auto statevar = dynamic_cast<fs::StateVariable::cptr>(effect->lhs())) {
		_unused(statevar);
		assert(statevar->getValue() == atom.getVariable()); // Otherwise we shouldn't be considering this effect as a potential achiever of atom.
	} else if (auto nested = dynamic_cast<fs::FluentHeadedNestedTerm::cptr>(effect->lhs())) {
		_unused(nested);
		const auto& data = info.getVariableData(atom.getVariable());
		assert(nested->getSymbolId() == data.first);
		assert(_lhs_subterm_variables.size() == data.second.size());
		for (unsigned i = 0; i < _lhs_subterm_variables.size(); ++i) {
			auto subterm_variable = _translator.resolveVariableFromIndex(_lhs_subterm_variables[i], csp);
			Gecode::rel(csp, subterm_variable,  Gecode::IRT_EQ, data.second[i]);
		}
		
	} else throw std::runtime_error("Unknown effect type");
	
	// This is equivalent, but faster, to _translator.resolveVariable(effect->rhs(), csp);
	assert(effect_rhs_variables.size()==1);
	auto& rhs_term =_translator.resolveVariableFromIndex(effect_rhs_variables[0], csp);
	Gecode::rel(csp, rhs_term,  Gecode::IRT_EQ, atom.getValue());
}

std::vector<unsigned> GroundEffectCSP::index_lhs_subterms() {
	std::vector<unsigned> subterm_variables;
	auto lhs = get_effect()->lhs();
	if (auto nested = dynamic_cast<fs::FluentHeadedNestedTerm::cptr>(lhs)) {
		for (auto subterm:nested->getSubterms()) {
			subterm_variables.push_back(_translator.resolveVariableIndex(subterm));
		}
	}
	return subterm_variables;
}

const fs::Formula* GroundEffectCSP::get_precondition() const {
	return _action.getPrecondition();
}

const std::vector<const fs::ActionEffect*>& GroundEffectCSP::get_effects() const {
	return _effects;
}

} } // namespaces
