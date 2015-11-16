
#include <constraints/gecode/handlers/schema_handler.hxx>
#include <utils/logging.hxx>
#include <actions/action_id.hxx>

namespace fs0 { namespace gecode {

std::vector<std::shared_ptr<BaseActionCSPHandler>> ActionSchemaCSPHandler::create(const std::vector<const ActionSchema*>& schemata) {
	std::vector<std::shared_ptr<BaseActionCSPHandler>> managers;
	
	bool use_novelty_constraint = Config::instance().useNoveltyConstraint();
	bool approximate = Config::instance().useApproximateActionResolution();
	
	for (auto schema:schemata) {
		auto manager = std::make_shared<ActionSchemaCSPHandler>(*schema, approximate, use_novelty_constraint);
		FDEBUG("main", "Generated CSP for action schema" << *schema << std::endl <<  *manager << std::endl);
		managers.push_back(manager);
	}
	return managers;
}

ActionSchemaCSPHandler::ActionSchemaCSPHandler(const ActionSchema& action, const std::vector<fs::ActionEffect::cptr>& effects, bool approximate, bool use_novelty_constraint)
:  BaseActionCSPHandler(action, effects, approximate, use_novelty_constraint)
{
	index_parameters();
}

ActionSchemaCSPHandler::ActionSchemaCSPHandler(const ActionSchema& action, bool approximate, bool use_novelty_constraint)
:  ActionSchemaCSPHandler(action,  action.getEffects(), approximate, use_novelty_constraint)
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

const ActionID* ActionSchemaCSPHandler::get_action_id(SimpleCSP* solution) const {
	return new LiftedActionID(_action.getId(), build_binding_from_solution(solution));
}

void ActionSchemaCSPHandler::log() const {
	FFDEBUG("heuristic", "Processing action schema #" << _action.getId() << ": " << _action.fullname());
}
} } // namespaces
