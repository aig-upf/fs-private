
#include <languages/fstrips/language.hxx>
#include <constraints/gecode/handlers/ground_effect_handler.hxx>
#include <constraints/gecode/helper.hxx>
#include <actions/actions.hxx>
#include <utils/printers/actions.hxx>
#include <utils/logging.hxx>
#include <actions/action_id.hxx>
#include <problem.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <state.hxx>
#include <gecode/driver.hh>

namespace fs0 { namespace gecode {

std::vector<std::shared_ptr<BaseActionCSPHandler>> GroundEffectCSPHandler::create(const std::vector<const GroundAction*>& actions, const TupleIndex& tuple_index, bool approximate, bool novelty) {
	std::vector<std::shared_ptr<BaseActionCSPHandler>> managers;
	
	for (unsigned action_idx = 0; action_idx < actions.size(); ++action_idx) {
		const auto action = actions[action_idx];
		
		for (unsigned eff_idx = 0; eff_idx < action->getEffects().size(); ++eff_idx) {
			auto handler = std::make_shared<GroundEffectCSPHandler>(*action, tuple_index, eff_idx, approximate);
			if (handler->init(novelty)) {
				managers.push_back(handler);
				FDEBUG("main", "Generated CSP for the effect #" << eff_idx << " of action " << print::action_header(*action) << std::endl <<  *handler << std::endl);
			} else {
				FDEBUG("main", "CSP for action effect " << action->getEffects().at(eff_idx) << " is inconsistent ==> the action is not applicable");
			}
		}
	}
	return managers;
}

GroundEffectCSPHandler::GroundEffectCSPHandler(const GroundAction& action, const TupleIndex& tuple_index, unsigned effect_idx, bool approximate) :
	BaseActionCSPHandler(action, { action.getEffects().at(effect_idx) }, tuple_index, approximate)
{}

bool GroundEffectCSPHandler::init(bool use_novelty_constraint) {
	if (!BaseActionCSPHandler::init(use_novelty_constraint)) return false;
	_lhs_subterm_variables = index_lhs_subterms();
	return true;
}

void GroundEffectCSPHandler::log() const {
	FFDEBUG("heuristic", "Processing effect \"" << *get_effect() << "\" of action " << _action);
}

const ActionID* GroundEffectCSPHandler::get_action_id(const SimpleCSP* solution) const {
	return new PlainActionID(static_cast<const GroundAction*>(&_action)); // TODO Ugly
}

SimpleCSP* GroundEffectCSPHandler::preinstantiate(const GecodeRPGLayer& layer) const {
	SimpleCSP* csp = instantiate_csp(layer);
	if (!csp) return nullptr;
	
	if (!csp->checkConsistency()) { // This colaterally enforces propagation of constraints
		delete csp;
		return nullptr;
	}
	
	return csp;
}

bool GroundEffectCSPHandler::find_atom_support(const Atom& atom, const State& seed, SimpleCSP& layer_csp, RPGData& rpg) const {
	log();
	
	std::unique_ptr<SimpleCSP> csp = std::unique_ptr<SimpleCSP>(static_cast<SimpleCSP*>(layer_csp.clone()));
	
	post(*csp, atom);

	if (!csp->checkConsistency()) { // This colaterally enforces propagation of constraints
		FFDEBUG("heuristic", "Action CSP inconsistent => atom " << atom << " cannot be derived through it");
		return false;
	} 
	
	// Else, the CSP is locally consistent
	if (_approximate) {  // Check only local consistency
		solve_approximately(atom, csp.get(), rpg, seed);
		return true;
	} else { // Else, we want a full solution of the CSP
		return solve_completely(csp.get(), rpg);
	}
}

bool GroundEffectCSPHandler::solve_completely(gecode::SimpleCSP* csp, RPGData& rpg) const {
	// We just want to search for one solution an extract the support from it
	Gecode::DFS<SimpleCSP> engine(csp);
	SimpleCSP* solution = engine.next();
	if (!solution) return false; // The CSP has no solution at all
	
	process_solution(solution, rpg); // TODO - This is not optimal, but for the moment being it saves us a lot of code duplication

	delete solution;
	return true;
}

void GroundEffectCSPHandler::solve_approximately(const Atom& atom, gecode::SimpleCSP* csp, RPGData& rpg, const State& seed) const {
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

		
		const ProblemInfo& info = Problem::getInfo();
		
		for (fs::FluentHeadedNestedTerm::cptr fluent:nested_terms) {
			VariableIdx variable = info.resolveStateVariable(fluent->getSymbolId(), _translator.resolveValues(fluent->getSubterms(), CSPVariableType::Input, *csp));
	//		VariableIdx variable = fluent->interpretVariable(assignment, binding);
			if (inserted.find(variable) == inserted.end()) { // Don't push twice the support the same atom
				// ObjectIdx value = fluent->interpret(assignment, binding);
				
				ObjectIdx value = 1; // i.e. assuming that there are no negated atoms on conditions.
				if (!info.isPredicate(fluent->getSymbolId())) {
					value = _translator.resolveValue(fluent, CSPVariableType::Input, *csp);
				}
				
				support->push_back(Atom(variable, value));
				inserted.insert(variable);
			}
		}
	}
	
	rpg.add(atom, get_action_id(csp), support);
}



void GroundEffectCSPHandler::post(SimpleCSP& csp, const Atom& atom) const {
	const ProblemInfo& info = Problem::getInfo();
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
	
	// This is equivalent, but faster, to _translator.resolveVariable(effect->rhs(), CSPVariableType::Input, csp);
	assert(effect_rhs_variables.size()==1);
	auto& rhs_term =_translator.resolveVariableFromIndex(effect_rhs_variables[0], csp);
	Gecode::rel(csp, rhs_term,  Gecode::IRT_EQ, atom.getValue());
}

std::vector<unsigned> GroundEffectCSPHandler::index_lhs_subterms() {
	std::vector<unsigned> subterm_variables;
	auto lhs = get_effect()->lhs();
	if (auto nested = dynamic_cast<fs::FluentHeadedNestedTerm::cptr>(lhs)) {
		for (auto subterm:nested->getSubterms()) {
			subterm_variables.push_back(_translator.resolveVariableIndex(subterm, CSPVariableType::Input));
		}
	}
	return subterm_variables;
}

} } // namespaces
