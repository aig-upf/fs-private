

class ComponentFactory : public BaseComponentFactory {
public:
    Action::cptr instantiateAction(
        const std::string& classname,
		const ObjectIdxVector& binding,
		const ObjectIdxVector& derived,
        const std::vector<VariableIdxVector>& appRelevantVars,
        const std::vector<VariableIdxVector>& effRelevantVars,
        const VariableIdxVector& effAffectedVars
    ) const {
        Action* pointer = nullptr;

        const ObjectIdxVector& _binding = binding; // syntactic sugar

        if (false) {}
        ${actions}
        else throw std::runtime_error("Unknown action name.");

        return pointer;
    }

    ScopedConstraint::vcptr instantiateGoal(const std::vector<VariableIdxVector>& appRelevantVars) const {
    const ObjectIdxVector binding;  // The goal has empty binding
		return {
		    ${goal_constraint_instantiations}
		};
    }

    ScopedConstraint::cptr instantiateConstraint(const std::string& classname, const ObjectIdxVector& parameters, const VariableIdxVector& variables) const {
        return external->instantiateConstraint(classname, parameters, variables);
    }

    std::map<std::string, Function> instantiateFunctions() const {
		return {
			{$functions}
		};
	}
};