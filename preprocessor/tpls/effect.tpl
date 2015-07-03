
class ${classname} : public ${parent} {
public:
	${classname}(const VariableIdxVector& scope, VariableIdx affected, const std::vector<int>& parameters) : ${parent}(scope, affected, parameters) {}

	Atom ${apply_header}
		${code}
	}

	virtual std::ostream& print(std::ostream& os) const {
		${getname_code}
		return os;
	}
};
