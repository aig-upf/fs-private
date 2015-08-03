

class ComponentFactory : public BaseComponentFactory {
public:
    GroundAction::cptr instantiateAction(
        const std::string& classname,
		const ObjectIdxVector& binding,
		const ObjectIdxVector& derived,
        const std::vector<VariableIdxVector>& appRelevantVars,
        const std::vector<VariableIdxVector>& effRelevantVars,
        const VariableIdxVector& effAffectedVars
    ) const {
        GroundAction* pointer = nullptr;

        // const ObjectIdxVector& _binding = binding; // syntactic sugar

        if (false) {}
        ${actions}
        else throw std::runtime_error("Unknown action name.");

        return pointer;
    }

    /*
    DirectConstraint::cptr instantiateConstraint(const std::string& classname, const ObjectIdxVector& parameters, const VariableIdxVector& variables) const {
        return external->instantiateConstraint(classname, parameters, variables);
    }
    */

    std::map<std::string, Function> instantiateFunctions() const {
		return {
			${functions}
		};
	}
};