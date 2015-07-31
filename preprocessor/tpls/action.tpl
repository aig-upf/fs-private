/*
class $classname : public Action {

protected:
    static const std::string name_;
    static const ActionSignature signature_;

public:
	$classname(const ObjectIdxVector& binding, const ScopedConstraint::vcptr& constraints, const ScopedEffect::vcptr& effects) : Action(binding, constraints, effects) {}

    const std::string& getName() const { return name_; }
    const ActionSignature& getSignature() const { return signature_; }
};
*/