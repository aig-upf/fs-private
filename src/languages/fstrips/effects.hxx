
#pragma once

#include <fs0_types.hxx>
#include <atoms.hxx>
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
	
	ActionEffect(Term::cptr lhs_, Term::cptr rhs_)
		: _lhs(lhs_), _rhs(rhs_) {
		if (!isWellFormed()) throw std::runtime_error("Ill-formed effect");
	}
	
	virtual ~ActionEffect() {
		delete _lhs; delete _rhs;
	}
	
	//! Checks that the 
	bool isWellFormed() const;
	
	//! Returns a vector with all the terms involved in the effect (possibly with repetitions)
	std::vector<Term::cptr> flatten() const;
	
	//! Applies the effect to the given state and returns the resulting atom
	Atom apply(const State& state) const;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ActionEffect& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;

	//! Accessors for the left-hand side and right-hand side of the effect
	const Term::cptr lhs() const { return _lhs; }
	const Term::cptr rhs() const { return _rhs; }
	
protected:
	Term::cptr _lhs;
	Term::cptr _rhs;
};

} } } // namespaces
