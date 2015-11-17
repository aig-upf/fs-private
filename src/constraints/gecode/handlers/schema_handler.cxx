
#include <constraints/gecode/handlers/schema_handler.hxx>
#include <utils/logging.hxx>
#include <actions/action_id.hxx>

namespace fs0 { namespace gecode {

std::vector<std::shared_ptr<BaseActionCSPHandler>> ActionSchemaCSPHandler::create(const std::vector<const ActionSchema*>& schemata, bool approximate, bool novelty, bool dont_care) {
	// Simply upcast the shared_ptrs
	std::vector<std::shared_ptr<BaseActionCSPHandler>> managers;
	for (const auto& element:create_derived(schemata, approximate, novelty, dont_care)) {
		managers.push_back(std::static_pointer_cast<BaseActionCSPHandler>(element));
	}
	return managers;
}

std::vector<std::shared_ptr<ActionSchemaCSPHandler>> ActionSchemaCSPHandler::create_derived(const std::vector<const ActionSchema*>& schemata, bool approximate, bool novelty, bool dont_care) {
	std::vector<std::shared_ptr<ActionSchemaCSPHandler>> managers;
	
	for (auto schema:schemata) {
		auto manager = std::make_shared<ActionSchemaCSPHandler>(*schema, approximate, novelty, dont_care);
		FDEBUG("main", "Generated CSP for action schema" << *schema << std::endl <<  *manager << std::endl);
		managers.push_back(manager);
	}
	return managers;
}


ActionSchemaCSPHandler::ActionSchemaCSPHandler(const ActionSchema& action, const std::vector<fs::ActionEffect::cptr>& effects, bool approximate, bool novelty, bool dont_care)
:  BaseActionCSPHandler(action, effects, approximate, novelty, dont_care)
{
	index_parameters();
}

ActionSchemaCSPHandler::ActionSchemaCSPHandler(const ActionSchema& action, bool approximate, bool novelty, bool dont_care)
:  ActionSchemaCSPHandler(action,  action.getEffects(), approximate, novelty, dont_care)
{}


void ActionSchemaCSPHandler::index_parameters() {
	// Index in '_parameter_variables' the (ordered) CSP variables that correspond to the action parameters
	const ActionSchema& schema = static_cast<const ActionSchema&>(_action);
	const Signature& signature = schema.getSignature();
	for (unsigned i = 0; i< signature.size(); ++i) {
		// We here assume that the parameter IDs are always 0..k-1, where k is the number of parameters
		fs::BoundVariable variable(i, signature[i]);
		_parameter_variables.push_back(_translator.resolveVariableIndex(&variable, CSPVariableType::Input));
	}
}

Binding ActionSchemaCSPHandler::build_binding_from_solution(SimpleCSP* solution) const {
	std::vector<int> values;
	values.reserve(_parameter_variables.size());
	for (unsigned csp_var_idx:_parameter_variables) {
		values.push_back(_translator.resolveValueFromIndex(csp_var_idx, *solution));
	}
	return Binding(values);
}

// Simply forward to the more concrete method
const ActionID* ActionSchemaCSPHandler::get_action_id(SimpleCSP* solution) const {
	return get_lifted_action_id(solution);
}

LiftedActionID* ActionSchemaCSPHandler::get_lifted_action_id(SimpleCSP* solution) const {
	return new LiftedActionID(_action.getId(), build_binding_from_solution(solution));
}

void ActionSchemaCSPHandler::log() const {
	FFDEBUG("heuristic", "Processing action schema #" << _action.getId() << ": " << _action.fullname());
}
} } // namespaces
