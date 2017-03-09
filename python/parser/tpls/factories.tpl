

class ComponentFactory : public BaseComponentFactory {
    std::map<std::string, Function> instantiateFunctions(const ProblemInfo& info) const override {
		return {
			${functions}
		};
	}
};