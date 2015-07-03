
class ${classname} : public ${parent} {
public:
	${classname}(const VariableIdxVector& scope, const std::vector<int>& parameters) : ${parent}(scope, parameters) {}

    bool ${satisfied_header} const {
		${code}
	}
	
	virtual std::ostream& print(std::ostream& os) const {
		${getname_code}
		return os;
	}
};
