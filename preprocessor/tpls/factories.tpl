

class ComponentFactory {
public:
    static Action::cptr instantiateAction(
        const std::string& classname,
		const ObjectIdxVector& binding,
		const ObjectIdxVector& derived,
        const std::vector<VariableIdxVector>& appRelevantVars,
        const std::vector<VariableIdxVector>& effRelevantVars,
        const VariableIdxVector& effAffectedVars
    ) {
        Action* pointer = nullptr;

        if (false) {}
        ${actions}
        else throw std::runtime_error("Unknown action name.");

        return pointer;
    }

    static ScopedConstraint::vcptr instantiateGoal(const std::vector<VariableIdxVector>& appRelevantVars) {
    const ObjectIdxVector binding;  // The goal has empty binding
		return {
		    ${goal_constraint_instantiations}
		};
    }

    static ScopedConstraint::cptr instantiateConstraint(const std::string& classname, const ObjectIdxVector& parameters, const VariableIdxVector& variables) {
        return external->instantiateConstraint(classname, parameters, variables);
    }
};