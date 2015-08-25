
#include <constraints/gecode/handlers/csp_handler.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/helper.hxx>
#include <heuristics/rpg_data.hxx>
#include <utils/logging.hxx>

#include <gecode/driver.hh>

namespace fs0 { namespace gecode {

GecodeActionCSPHandler::GecodeActionCSPHandler(const GroundAction& action)
	:  GecodeCSPHandler(),
	   _action(action)
{
	FDEBUG( "translation", "Gecode Action Handler: Processing Action " << _action.getFullName() << std::endl);
	createCSPVariables();

	registerFormulaConstraints(_action.getConditions());

	for (const auto effect:_action.getEffects()) {
		registerEffectConstraints(effect);
	}

	Helper::postBranchingStrategy(_base_csp);

	// MRJ: in order to be able to clone a CSP, we need to ensure that it is "stable" i.e. propagate all constraints until fixed point
	Gecode::SpaceStatus st = _base_csp.status();

	assert(st != Gecode::SpaceStatus::SS_FAILED); // This should never happen, as it means that the action is (statically) unapplicable.
}


SimpleCSP::ptr GecodeActionCSPHandler::instantiate_csp(const RelaxedState& layer) const {
	// MRJ: This is rather ugly, the problem is that clone returns a pointer to Space...
	// it may be a good idea to overwrite this method on SimpleCSP to avoid this downcast.
	// GFM: Can probably be safely changed to a static cast??
	// MRJ: Hmmmm...
	SimpleCSP* csp = dynamic_cast<SimpleCSP::ptr>(_base_csp.clone());
	_translator.updateStateVariableDomains(*csp, layer);
	return csp;
}

void GecodeActionCSPHandler::print_csp( std::ostream& os, SimpleCSP::ptr csp ) const {
	// MRJ: We simple forward the call to the translator, who knows the names of the variables
	_translator.print( os, *csp );
}


void GecodeActionCSPHandler::createCSPVariables() {
	// Determine input and output variables for this action: we first amalgamate variables into a set
	// to avoid repetitions, then generate corresponding CSP variables, then create the CSP model with them
	// and finally add the model constraints.

	IntVarArgs variables;

	registerFormulaVariables(_action.getConditions(), variables);
	for (const auto effect:_action.getEffects()) {
		registerEffectVariables(effect, variables);
	}

	IntVarArray tmp(_base_csp, variables);
	_base_csp._X.update(_base_csp, false, tmp);
}

void GecodeActionCSPHandler::registerEffectVariables(const fs::ActionEffect::cptr effect, Gecode::IntVarArgs& variables) {
	// Register first the RHS variables as input variables
	registerTermVariables(effect->rhs, CSPVariableType::Input, _base_csp, _translator, variables);

	// As for the LHS variable, we register the root level as an output CSP variable, and the children (if any) recursively as input variables
	registerTermVariables(effect->lhs, CSPVariableType::Output, _base_csp, _translator, variables);
}


void GecodeActionCSPHandler::registerEffectConstraints(const fs::ActionEffect::cptr effect) {
	// Register the lhs and rhs constraints recursively
	GecodeCSPHandler::registerTermConstraints(effect->lhs, CSPVariableType::Output, _base_csp, _translator);
	GecodeCSPHandler::registerTermConstraints(effect->rhs, CSPVariableType::Input, _base_csp, _translator);

	// And now equate the output variable corresponding to the LHS term with the input variable corresponding to the RHS term
	const Gecode::IntVar& lhs_gec_var = _translator.resolveVariable(effect->lhs, CSPVariableType::Output, _base_csp);
	const Gecode::IntVar& rhs_gec_var = _translator.resolveVariable(effect->rhs, CSPVariableType::Input, _base_csp);
	Gecode::rel(_base_csp, lhs_gec_var, Gecode::IRT_EQ, rhs_gec_var);
}


void GecodeActionCSPHandler::compute_support(gecode::SimpleCSP* csp, unsigned actionIdx, RPGData& rpg) const {
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	unsigned num_solutions = 0;
	DFS<SimpleCSP> engine(csp);
	FFDEBUG("heuristic", "Computing supports for action " << _action.getFullName());
	while (SimpleCSP* solution = engine.next()) {
		FFDEBUG("heuristic", std::endl << "Processing action CSP solution: " << *solution);
		
		for (ActionEffect::cptr effect : _action.getEffects()) {
			VariableIdx affected;
			assert(effect->affected.size() > 0);
			
			if (dynamic_cast<fs::StateVariable::cptr>(effect->lhs)) {
				assert(effect->affected.size() == 1);
				affected = effect->affected[0];
			} else { // We necessarily have a nested fluent on the LHS of the effect
				auto fluent = dynamic_cast<fs::FluentHeadedNestedTerm::cptr>(effect->lhs);
				assert(fluent && fluent->getSubterms().size() == 1);
				ObjectIdx index_val = _translator.resolveValue(fluent->getSubterms()[0], CSPVariableType::Input, *solution);
				affected = info.resolveStateVariable(fluent->getSymbolId(), {index_val});
			}

			Atom atom(affected, _translator.resolveOutputStateVariableValue(*solution, affected)); // TODO - this might be optimized and factored out of the loop?
			auto hint = rpg.getInsertionHint(atom);
			FFDEBUG("heuristic", "Processing effect \"" << *effect << "\" (" << effect->affected.size() << " affected variables) yields new atom " << atom);

			if (hint.first) { // The value is actually new - let us compute the supports, i.e. the CSP solution values for each variable relevant to the effect.
				Atom::vctrp atomSupport = std::make_shared<Atom::vctr>();
				Atom::vctrp actionSupport = std::make_shared<Atom::vctr>();
				boost::container::flat_set<VariableIdx> processed;

				// TODO - The set of variables related to the support of each effect could indeed be precomputed in order to speed up this.
				// First extract the supports related to the variables relevant to the particular effect
				for (VariableIdx variable: effect->scope) {
					atomSupport->push_back(Atom(variable, _translator.resolveInputStateVariableValue(*solution, variable)));
					processed.insert(variable);
				}

				// And now those related to the variables relevant to the action precondition - without repeating those that might already have been part of the effect.
				for (VariableIdx variable: _action.getScope()) {
					if (processed.find(variable) == processed.end()) {
						actionSupport->push_back(Atom(variable, _translator.resolveInputStateVariableValue(*solution, variable)));
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
	assert(0);
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
