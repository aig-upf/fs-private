
#include <languages/fstrips/terms.hxx>
#include <constraints/gecode/handlers/action_schema_handler.hxx>
#include <actions/actions.hxx>
#include <aptk2/tools/logging.hxx>
#include <actions/action_id.hxx>

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
		auto handler = std::make_shared<ActionSchemaCSPHandler>(*schema, tuple_index, approximate);
		handler->init(novelty);
		LPT_DEBUG("main", "Generated CSP for action schema " << *schema << std::endl <<  *handler << std::endl);
		handlers.push_back(handler);
	}
	return handlers;
}


ActionSchemaCSPHandler::ActionSchemaCSPHandler(const PartiallyGroundedAction& action, const TupleIndex& tuple_index, bool approximate)
:  BaseActionCSPHandler(tuple_index, approximate), _action(action)
{}


bool ActionSchemaCSPHandler::init(bool use_novelty_constraint) {
	if (BaseActionCSPHandler::init(use_novelty_constraint)) {
		index_parameters();
		return true;
	}
	return false;
}


void ActionSchemaCSPHandler::index_parameters() {
	// Index in '_parameter_variables' the (ordered) CSP variables that correspond to the action parameters
	const Signature& signature = _action.getSignature();
	for (unsigned i = 0; i < signature.size(); ++i) {
		if (_action.isBound(i)) {
			// If the parameter is bound, we mark it specially so that we know the value cannot be retrieved from the CSP,
			// because there will be no CSP variable modelling that parameter
			_parameter_variables.push_back(std::numeric_limits<unsigned int>::max());
		} else {
			// We here assume that the parameter IDs are always 0..k-1, where k is the number of parameters
			fs::BoundVariable variable(i, signature[i]);
			_parameter_variables.push_back(_translator.resolveVariableIndex(&variable));
		}
	}
}

Binding ActionSchemaCSPHandler::build_binding_from_solution(const SimpleCSP* solution) const {
	std::vector<int> values;
	std::vector<bool> valid;
	values.reserve(_parameter_variables.size());
	valid.reserve(_parameter_variables.size());
	for (unsigned csp_var_idx:_parameter_variables) {
		if (csp_var_idx == std::numeric_limits<unsigned int>::max()) {
			values.push_back(0);
			valid.push_back(false);
		} else {
			values.push_back(_translator.resolveValueFromIndex(csp_var_idx, *solution));
			valid.push_back(true);
		}
	}
	return Binding(std::move(values), std::move(valid));
}

const std::vector<const fs::ActionEffect*>& ActionSchemaCSPHandler::get_effects() const {
	return _action.getEffects();
}

const fs::Formula* ActionSchemaCSPHandler::get_precondition() const {
	return _action.getPrecondition();
}

// Simply forward to the more concrete method
const ActionID* ActionSchemaCSPHandler::get_action_id(const SimpleCSP* solution) const {
	return get_lifted_action_id(solution);
}

LiftedActionID* ActionSchemaCSPHandler::get_lifted_action_id(const SimpleCSP* solution) const {
	return new LiftedActionID(&_action, build_binding_from_solution(solution));
}

void ActionSchemaCSPHandler::log() const {
	LPT_EDEBUG("heuristic", "Processing action schema " << _action);
}
} } // namespaces
