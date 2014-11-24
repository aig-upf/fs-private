
class $actionName : public CoreAction {

protected:
    static const std::string name_;
    static const ActionSignature signature_;

public:
    $constructor

    virtual const std::string& getName() const { return name_; }
    virtual const ActionSignature& getSignature() const { return signature_; }

    virtual bool isApplicable(unsigned procedureIdx, const ProcedurePoint& relevant) const {
		$applicability_code_switch
		throw std::runtime_error("Shouldn't reach this point - wrong procedure index");
	}

	void applyEffectProcedure(unsigned procedureIdx, const ProcedurePoint& relevant, ProcedurePoint& affected) const {
	    $effect_code_switch
	}
};
