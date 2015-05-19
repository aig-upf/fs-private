
class ${classname} : public ${parent} {
public:
	${classname}(const VariableIdxVector& scope, const std::vector<int>& parameters) : ${parent}(scope, parameters) {}

    bool ${satisfied_header} const {
		${code}
	}
	
	std::string getName() const {
		${getname_code}
	}
};
