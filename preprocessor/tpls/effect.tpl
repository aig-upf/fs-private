
class ${classname} : public ${parent} {
public:
	${classname}(const VariableIdxVector& scope, const VariableIdxVector& image, const std::vector<int>& parameters) : ${parent}(scope, image, parameters) {}

	ObjectIdx apply(const ObjectIdxVector& relevant) const {
		${code}
	}
};
