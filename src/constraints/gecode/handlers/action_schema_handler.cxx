
#include <languages/fstrips/terms.hxx>
#include <constraints/gecode/handlers/action_schema_handler.hxx>
#include <actions/actions.hxx>
#include <utils/logging.hxx>
#include <actions/action_id.hxx>
#include <utils/binding.hxx>

namespace fs0 { namespace gecode {

std::vector<std::shared_ptr<BaseActionCSPHandler>> ActionSchemaCSPHandler::create(const std::vector<const PartiallyGroundedAction*>& schemata, const TupleIndex& tuple_index, bool approximate, bool novelty) {
	// Simply upcast the shared_ptrs
	std::vector<std::shared_ptr<BaseActionCSPHandler>> handlers;
	for (const auto& element:create_derived(schemata, tuple_index, approximate, novelty)) {
		handlers.push_back(std::static_pointer_cast<BaseActionCSPHandler>(element));
	}
	return handlers;
}

std::vector<std::shared_ptr<ActionSchemaCSPHandler>> ActionSchemaCSPHandler::create_derived(const std::vector<const PartiallyGroundedAction*>& schemata, const TupleIndex& tuple_index, bool approximate, bool novelty) {
	std::vector<std::shared_ptr<ActionSchemaCSPHandler>> handlers;
	
	for (auto schema:schemata) {
		auto handler = std::make_shared<ActionSchemaCSPHandler>(*schema, schema->getEffects(), tuple_index, approximate);
		handler->init(novelty);
		FDEBUG("main", "Generated CSP for action schema" << *schema << std::endl <<  *handler << std::endl);
		handlers.push_back(handler);
	}
	return handlers;
}


ActionSchemaCSPHandler::ActionSchemaCSPHandler(const PartiallyGroundedAction& action, const std::vector<const fs::ActionEffect*>& effects, const TupleIndex& tuple_index, bool approximate)
:  BaseActionCSPHandler(action, effects, tuple_index, approximate)
{}


void ActionSchemaCSPHandler::init(bool use_novelty_constraint) {
	BaseActionCSPHandler::init(use_novelty_constraint);
	index_parameters();
}


void ActionSchemaCSPHandler::index_parameters() {
	// Index in '_parameter_variables' the (ordered) CSP variables that correspond to the action parameters
	const Signature& signature = _action.getSignature();
	for (unsigned i = 0; i < signature.size(); ++i) {
		if (_action.isBound(i)) continue; // The parameter is bound
		// We here assume that the parameter IDs are always 0..k-1, where k is the number of parameters
		fs::BoundVariable variable(i, signature[i]);
		_parameter_variables.push_back(_translator.resolveVariableIndex(&variable, CSPVariableType::Input));
	}
}

Binding ActionSchemaCSPHandler::build_binding_from_solution(const SimpleCSP* solution) const {
	std::vector<int> values;
	values.reserve(_parameter_variables.size());
	for (unsigned csp_var_idx:_parameter_variables) {
		values.push_back(_translator.resolveValueFromIndex(csp_var_idx, *solution));
	}
	return Binding(values);
}

// Simply forward to the more concrete method
const ActionID* ActionSchemaCSPHandler::get_action_id(const SimpleCSP* solution) const {
	return get_lifted_action_id(solution);
}

LiftedActionID* ActionSchemaCSPHandler::get_lifted_action_id(const SimpleCSP* solution) const {
	// TODO - VERY UGLY
	return new LiftedActionID(static_cast<const PartiallyGroundedAction*>(&_action), build_binding_from_solution(solution));
}

void ActionSchemaCSPHandler::log() const {
	FFDEBUG("heuristic", "Processing action schema #" << _action.getId() << ": " << _action);
}
} } // namespaces
