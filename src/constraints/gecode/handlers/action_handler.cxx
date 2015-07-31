
#include <constraints/gecode/handlers/csp_handler.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/helper.hxx>
#include <heuristics/rpg_data.hxx>
#include <utils/logging.hxx>

#include <gecode/driver.hh>

namespace fs0 { namespace gecode {

GecodeActionCSPHandler::GecodeActionCSPHandler(const GroundAction& action)
	:  GecodeCSPHandler(),
	   _action(action),
	   actionTranslator(action, _base_csp, translator)
{
	createCSPVariables();
	registerConstraints();
	Helper::postBranchingStrategy(_base_csp);
	
	// MRJ: in order to be able to clone a CSP, we need to ensure that it is "stable" i.e. propagate all constraints until fixed point
	Gecode::SpaceStatus st = _base_csp.status();
	
	assert(st != Gecode::SpaceStatus::SS_FAILED); // This should never happen, as it means that the action is (statically) unapplicable.
}


SimpleCSP::ptr GecodeActionCSPHandler::instantiate_csp(const RelaxedState& layer) const {
	// MRJ: This is rather ugly, the problem is that clone returns a pointer to Space...
	// it may be a good idea to overwrite this method on SimpleCSP to avoid this downcast.
	// GFM: Can probably be safely changed to a static cast??
	SimpleCSP* csp = dynamic_cast<SimpleCSP::ptr>(_base_csp.clone());
	Helper::addRelevantVariableConstraints(*csp, translator, _action.getAllRelevantVariables(), layer);
	return csp;
}

void GecodeActionCSPHandler::createCSPVariables() {
	// Determine input and output variables for this action: we first amalgamate variables into a set
	// to avoid repetitions, then generate corresponding CSP variables, then create the CSP model with them
	// and finally add the model constraints.

	IntVarArgs variables;
	actionTranslator.registerVariables(variables);
	IntVarArray tmp(_base_csp, variables);
	_base_csp._X.update(_base_csp, false, tmp);
}


void GecodeActionCSPHandler::compute_support(gecode::SimpleCSP* csp, unsigned actionIdx, RPGData& rpg) const {
	unsigned num_solutions = 0;
	DFS<SimpleCSP> engine(csp);

	while (SimpleCSP* solution = engine.next()) {
		for (ActionEffect::cptr effect : _action.getEffects()) {
			
			// TODO - Handle nested effects possibly affecting different variables.
			// Do it intelligently: For each tuple in the solution of a first-level element constraint, actually only one variable will be affected!
			assert(effect->affected.size()==1);
			
			VariableIdx affected = effect->affected[0];
			Atom atom(affected, translator.resolveValue(*solution, affected, CSPVariableType::Output)); // TODO - this could be optimized and factored out of the loop
			auto hint = rpg.getInsertionHint(atom);
			
			if (hint.first) { // The value is actually new - let us compute the supports, i.e. the CSP solution values for each variable relevant to the effect.
				Atom::vctrp atomSupport = std::make_shared<Atom::vctr>();
				Atom::vctrp actionSupport = std::make_shared<Atom::vctr>();
				boost::container::flat_set<VariableIdx> processed;
				
				// TODO - The set of variables related to the support of each effect could indeed be precomputed in order to speed up this.
				// First extract the supports related to the variables relevant to the particular effect
				for (VariableIdx variable: effect->scope) {
					atomSupport->push_back(Atom(variable, translator.resolveValue(*solution, variable, CSPVariableType::Input)));
					processed.insert(variable);
				}
				
				// And now those related to the variables relevant to the action precondition - without repeating those that might already have been part of the effect.
				for (VariableIdx variable: _action.getScope()) {
					if (processed.find(variable) == processed.end()) {
						actionSupport->push_back(Atom(variable, translator.resolveValue(*solution, variable, CSPVariableType::Input)));
					}
				}
				
				// Once the support is computed, we insert the new atom into the RPG data structure
				rpg.add(atom, actionIdx, actionSupport, atomSupport, hint.second);
			}
		}
		++num_solutions;
		delete solution;
	}
	
	FDEBUG("main", "Solving the Action CSP completely produced " << num_solutions << " solutions"  << std::endl << *this);
}


void GecodeActionCSPHandler::registerConstraints() {
	// translateConstraints(_base_csp, translator, stateConstraints); // state constraints
	actionTranslator.registerConstraints();
}

/*
void GecodeActionCSPHandler::addNoveltyConstraints(const VariableIdx variable, const RelaxedState& layer, SimpleCSP& csp) {
	// TODO - This could be built incrementally to incorporate values added in this layer by previous actions in the iteration!
	auto& csp_var = translator.resolveVariable(csp, variable, CSPVariableType::Output);
	for (ObjectIdx value:*(layer.getValues(variable))) {
		rel( csp, csp_var, IRT_NQ, value ); // v != value
	}
}
*/

} } // namespaces
