
#pragma once

namespace fs0 { namespace language { namespace fstrips {

class Term;
class StateVariable; 

}}}
namespace fs = fs0::language::fstrips;

namespace fs0 {

class DirectEffect;

class EffectTranslator {
public:
	EffectTranslator() {}
	virtual ~EffectTranslator() {}

	//! Translate an effect of the form affected := rhs, where affected is constrained to be a state variable
	virtual const DirectEffect* translate(const fs::StateVariable& affected, const fs::Term& rhs) const = 0;
};


//! Translates effects where the RHS is a constant
class ConstantRhsTranslator : public EffectTranslator {
public:
	ConstantRhsTranslator() {}

	const DirectEffect* translate(const fs::StateVariable& affected, const fs::Term& rhs) const;
};

//! Translates effects where the RHS is a state variable
class StateVariableRhsTranslator : public EffectTranslator {
public:
	StateVariableRhsTranslator() {}

	const DirectEffect* translate(const fs::StateVariable& affected, const fs::Term& rhs) const;
};

//! Translates effects where the RHS is an addition
class AdditiveTermRhsTranslator : public EffectTranslator {
public:
	AdditiveTermRhsTranslator() {}

	const DirectEffect* translate(const fs::StateVariable& affected, const fs::Term& rhs) const;
};

//! Translates effects where the RHS is a subtraction
class SubtractiveTermRhsTranslator : public EffectTranslator {
public:
	SubtractiveTermRhsTranslator() {}

	const DirectEffect* translate(const fs::StateVariable& affected, const fs::Term& rhs) const;
};


//! Translates effects where the RHS is a multiplication
class MultiplicativeTermRhsTranslator : public EffectTranslator {
public:
	MultiplicativeTermRhsTranslator() {}

	const DirectEffect* translate(const fs::StateVariable& affected, const fs::Term& rhs) const;
};


} // namespaces
