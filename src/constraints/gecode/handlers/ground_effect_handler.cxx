
#include <languages/fstrips/language.hxx>
#include <constraints/gecode/handlers/ground_effect_handler.hxx>
#include <actions/ground_action.hxx>
#include <utils/printers/actions.hxx>
#include <utils/logging.hxx>
#include <actions/action_id.hxx>
#include <gecode/driver.hh>

namespace fs0 { namespace gecode {

std::vector<std::shared_ptr<BaseActionCSPHandler>> GroundEffectCSPHandler::create(const std::vector<const GroundAction*>& actions, bool approximate, bool novelty) {
	std::vector<std::shared_ptr<BaseActionCSPHandler>> managers;
	
	for (unsigned action_idx = 0; action_idx < actions.size(); ++action_idx) {
		const auto action = actions[action_idx];
		
		for (unsigned eff_idx = 0; eff_idx < action->getEffects().size(); ++eff_idx) {
			auto handler = std::make_shared<GroundEffectCSPHandler>(*action, eff_idx, approximate, novelty);
			managers.push_back(handler);
			FDEBUG("main", "Generated CSP for the effect #" << eff_idx << " of action " << print::action_name(*action) << std::endl <<  *handler << std::endl);
		}
	}
	return managers;
}

GroundEffectCSPHandler::GroundEffectCSPHandler(const GroundAction& action, unsigned effect_idx, bool approximate, bool novelty)
	: BaseActionCSPHandler(action, { action.getEffects().at(effect_idx) }, approximate, novelty)
{}

void GroundEffectCSPHandler::log() const {
	FFDEBUG("heuristic", "Processing effect \"" << *get_effect() << "\" of action " << _action.fullname());
}

const ActionID* GroundEffectCSPHandler::get_action_id(SimpleCSP* solution) const {
	return new PlainActionID(_action.getId());
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
		throw UnimplementedFeatureException("Approximate support not yet implemented in action CSPs");
	}
	
	// Else, we want a full solution of the CSP
	Gecode::DFS<SimpleCSP> engine(csp.get());
	SimpleCSP* solution = engine.next();
	if (!solution) { // The CSP has no solution at all
		return false;
	}
	
	// TODO - This is not optimal, but for the moment being it saves us a lot of code duplication
	process_solution(solution, rpg);

	delete solution;
	return true;
}


void GroundEffectCSPHandler::post(SimpleCSP& csp, const Atom& atom) const {
	assert(_effects.size() == 1);
	const auto& effect = _effects[0];
	auto statevar = dynamic_cast<fs::StateVariable::cptr>(effect->lhs());
	assert(statevar);
	assert(statevar->getValue() == atom.getVariable()); // Otherwise we shouldn't be considering this effect as a potential achiever of atom.
	
	// This is equivalent, but faster, to _translator.resolveVariable(effect->rhs(), CSPVariableType::Input, csp);
	assert(effect_rhs_variables.size()==1);
	auto& rhs_term =_translator.resolveVariableFromIndex(effect_rhs_variables[0], csp);
	
	Gecode::rel(csp, rhs_term,  Gecode::IRT_EQ, atom.getValue());
}

} } // namespaces
