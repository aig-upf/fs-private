
class ${classname} : public ${parent} {
public:
	${classname}(const VariableIdxVector& scope, const VariableIdxVector& image, const std::vector<int>& parameters) : ${parent}(scope, image, parameters) {}

	Atom ${apply_header}
		${code}
	}

	std::string getName() const { 
		${getname_code}
	}
};
