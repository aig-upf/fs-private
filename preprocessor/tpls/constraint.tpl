
class ${classname} : public ${parent} {
public:
	${classname}(const VariableIdxVector& scope, const std::vector<int>& parameters) : ${parent}(scope, parameters) {}

    bool isSatisfied(const ObjectIdxVector& relevant) const {
		${code}
	}
};