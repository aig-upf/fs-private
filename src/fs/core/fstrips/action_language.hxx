
#pragma once

#include <ostream>

#include <fs/core/fstrips/fol_syntax_fwd.hxx>
#include <fs/core/fs_types.hxx>

namespace fs0::fstrips {

class LanguageInfo;


//! The effect of a planning (grounded) action, which is of the form
//!     LHS := RHS
//! where both LHS (left-hand side) and RHS (right-hand side) are terms in our language,
//! with the particularity that LHS must be either a state variable or a fluent-headed nested term.
class ActionEffect {
public:
    explicit ActionEffect(const Formula* condition) : _condition(condition) {}
	virtual ~ActionEffect();
	
	ActionEffect(const ActionEffect&);
	ActionEffect(ActionEffect&&) = default;
	ActionEffect& operator=(const ActionEffect&) = delete;
	ActionEffect& operator=(ActionEffect&&) = default;
	
	virtual ActionEffect* clone() const = 0;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ActionEffect& o);
	virtual std::ostream& print(std::ostream& os, const LanguageInfo& info) const = 0;

	//! Accessor
	const Formula* condition() const { return _condition; }
	
	virtual bool is_predicative() const = 0;
	virtual bool is_add() const = 0;
	virtual bool is_del() const = 0;
	
protected:
	
	//! The effect condition
	const Formula* _condition;
};


class FunctionalEffect : public ActionEffect {
public:
	FunctionalEffect(const CompositeTerm* lhs, const Term* rhs, const Formula* condition)
		: ActionEffect(condition), _lhs(lhs), _rhs(rhs) {}
    ~FunctionalEffect() override;
	
	FunctionalEffect(const FunctionalEffect&);
	FunctionalEffect(FunctionalEffect&&) = default;
	FunctionalEffect& operator=(const FunctionalEffect&) = delete;
	FunctionalEffect& operator=(FunctionalEffect&&) = default;
	
	FunctionalEffect* clone() const override { return new FunctionalEffect(*this); }
	
	std::ostream& print(std::ostream& os, const LanguageInfo& info) const override;

	//! Accessors for the left-hand side and right-hand side of the effect
	const Term* lhs() const;
	const Term* rhs() const;
	
	bool is_predicative() const override { return false; };
	bool is_add() const override { return false; };
	bool is_del() const override { return false; };
	
protected:
	//! The LHS _must_ be a functional term, i.e. we cannot modify the
	//! denotation of (i.e. assign to) a constant or a logical variable
	const CompositeTerm* _lhs;
	
	//! The effect RHS
	const Term* _rhs;
};

enum class AtomicEffectType {ADD, DEL};

class AtomicEffect : public ActionEffect {
public:
	static AtomicEffectType to_type(const std::string& type);
	
	AtomicEffect(const AtomicFormula* atom, AtomicEffectType type, const Formula* condition)
		: ActionEffect(condition), _atom(atom), _type(type) {}
    ~AtomicEffect() override;
	
	AtomicEffect(const AtomicEffect&);
	AtomicEffect(AtomicEffect&&) = default;
	AtomicEffect& operator=(const AtomicEffect&) = delete;
	AtomicEffect& operator=(AtomicEffect&&) = default;
	
	AtomicEffect* clone() const override { return new AtomicEffect(*this); }
	
	std::ostream& print(std::ostream& os, const LanguageInfo& info) const override;

	const AtomicFormula* getAtom() const { return _atom; }
	const AtomicEffectType getType() const { return _type; }
	
	bool is_predicative() const override { return true; };
	bool is_add() const override { return _type == AtomicEffectType::ADD; }
	bool is_del() const override { return _type == AtomicEffectType::DEL; }
	
protected:
	//! An AtomicEffect involves an atomic formula
	const AtomicFormula* _atom;
	
	//! The type (add/del) of atomic effect
	const AtomicEffectType _type;
};



class ActionSchema {
protected:
	//! The ID of the original action schema (not to be confused with the ID of resulting fully-grounded actions)
	unsigned _id;
	
	const std::string _name;
	const Signature _signature;
	const std::vector<std::string> _parameter_names;
	const Formula* _precondition;
	const std::vector<const ActionEffect*> _effects;

public:
	ActionSchema(unsigned id, const std::string& name, const Signature& signature, const std::vector<std::string>& parameter_names,
                 const Formula* precondition, const std::vector<const ActionEffect*>& effects);
	
	~ActionSchema();
	ActionSchema(const ActionSchema&);

    ActionSchema* clone() const { return new ActionSchema(*this); }

    unsigned getId() const { return _id; }
	const std::string& getName() const { return _name; }
	const Signature& getSignature() const { return _signature; }
	const std::vector<std::string>& getParameterNames() const { return _parameter_names; }
	const Formula* getPrecondition() const { return _precondition; }
	const std::vector<const ActionEffect*>& getEffects() const { return _effects; }
	
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ActionSchema& entity) { return entity.print(os); }
	std::ostream& print(std::ostream& os) const;
};






} // namespaces
