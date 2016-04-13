
#pragma once

#include <fs_types.hxx>
#include <atom.hxx>
#include <problem_info.hxx>
#include <languages/fstrips/terms.hxx>

namespace fs0 { class State; }

namespace fs0 { namespace language { namespace fstrips {

//! The effect of a planning (grounded) action, which is of the form
//!     LHS := RHS
//! where both LHS (left-hand side) and RHS (right-hand side) are terms in our language,
//! with the particularity that LHS must be either a state variable or a fluent-headed nested term.
class ActionEffect {
public:
	typedef const ActionEffect* cptr;
	
	ActionEffect(Term::cptr lhs_, Term::cptr rhs_);
	
	virtual ~ActionEffect();
	
	ActionEffect(const ActionEffect& other);
	ActionEffect(ActionEffect&& other) = delete;
	ActionEffect& operator=(const ActionEffect& rhs) = delete;
	ActionEffect& operator=(ActionEffect&& rhs) = delete;
	
	
	//! Checks that the effect is well formed
	bool isWellFormed() const;
	
	ActionEffect::cptr bind(const Binding& binding, const ProblemInfo& info) const;


	//! Returns a vector with all the terms involved in the effect (possibly with repetitions)
	std::vector<Term::cptr> all_terms() const;
	
	//! Applies the effect to the given state and returns the resulting atom
	Atom apply(const State& state) const;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ActionEffect& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;

	//! Accessors for the left-hand side and right-hand side of the effect
	const Term::cptr lhs() const { return _lhs; }
	const Term::cptr rhs() const { return _rhs; }
	
	
	bool is_predicative() const;
	bool is_add() const;
	bool is_del() const;
	
protected:
	Term::cptr _lhs;
	Term::cptr _rhs;
};

} } } // namespaces
