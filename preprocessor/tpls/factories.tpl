

class ComponentFactory : public BaseComponentFactory {
    std::map<std::string, Function> instantiateFunctions() const {
		return {
			${functions}
		};
	}
};