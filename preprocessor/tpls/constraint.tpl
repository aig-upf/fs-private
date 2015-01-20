
class ${classname} : public UnaryExternalScopedConstraint {
public:
	${classname}(const VariableIdxVector& scope, const std::vector<int>& parameters) :
		UnaryExternalScopedConstraint(scope, parameters) {}

    bool isSatisfied(const ObjectIdxVector& relevant) const {
		${code}
	}
};