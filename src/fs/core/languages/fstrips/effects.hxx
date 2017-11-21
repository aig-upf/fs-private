
#pragma once

#include <fs/core/fs_types.hxx>
#include <fs/core/atom.hxx>
#include <fs/core/utils/binding.hxx>

namespace fs0 {
class ProblemInfo;
class State;
}

namespace fs0 { namespace language { namespace fstrips {

class Term;
class Formula;

//! The effect of a planning (grounded) action, which is of the form
//!     LHS := RHS
//! where both LHS (left-hand side) and RHS (right-hand side) are terms in our language,
//! with the particularity that LHS must be either a state variable or a fluent-headed nested term.
class ActionEffect {
public:
	ActionEffect(const Term* lhs, const Term* rhs, const Formula* condition);

	virtual ~ActionEffect();

	ActionEffect(const ActionEffect&);
	ActionEffect(ActionEffect&&) = default;
	ActionEffect& operator=(const ActionEffect&) = delete;
	ActionEffect& operator=(ActionEffect&&) = default;

	ActionEffect* clone() const { return new ActionEffect(*this); }

	//! Checks that the effect is well formed
	bool isWellFormed() const;

	//! Binds an effect schema with a particular binding.
	//! Might return null if the binding results in a non-applicable (conditional) effect
	const ActionEffect* bind(const Binding& binding, const ProblemInfo& info) const;


	//! Returns a vector with all the terms involved in the effect (possibly with repetitions)
	std::vector<const Term*> all_terms() const;

	//! Applies the effect to the given state and returns the resulting atom
	Atom apply(const State& state) const;

	//! Whether the effect is applicable in the given state. Non-conditional effects are always applicable.
	bool applicable(const State& state) const;

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ActionEffect& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;

	//! Accessors for the left-hand side and right-hand side of the effect
	const Term* lhs() const { return _lhs; }
	const Term* rhs() const { return _rhs; }
	const Formula* condition() const { return _condition; }

	bool is_predicative() const;
	bool is_add() const;
	bool is_del() const;

protected:

	//! The effect LHS
	const Term* _lhs;

	//! The effect RHS
	const Term* _rhs;

	//! The effect condition
	const Formula* _condition;
};

class ProceduralEffect {
	public:
	ProceduralEffect();
	virtual ~ProceduralEffect();

	ProceduralEffect(const ProceduralEffect&);
	ProceduralEffect(ProceduralEffect&&) = default;
	ProceduralEffect& operator=(const ProceduralEffect&) = delete;
	ProceduralEffect& operator=(ProceduralEffect&&) = default;

	virtual ProceduralEffect* clone() const = 0;

	void bind( const Binding& b ) { _binding = b; }

	//! Applies the effect to the given state and returns the resulting atom
	virtual void apply(const State& state, std::vector<Atom>& atoms) const = 0;

	//! Whether the effect is applicable in the given state. Non-conditional effects are always applicable.
	bool applicable(const State& state) const {return true;}

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ProceduralEffect& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;

	protected:

	//! The objects the effect is bound to
	Binding 	_binding;
};

} } } // namespaces
