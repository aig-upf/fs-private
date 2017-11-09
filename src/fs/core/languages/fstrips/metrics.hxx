
#pragma once

#include <fs/core/fs_types.hxx>
#include <fs/core/atom.hxx>
#include <fs/core/problem_info.hxx>
#include <fs/core/languages/fstrips/terms.hxx>

namespace fs0 { class State; }

namespace fs0 { namespace language { namespace fstrips {

//! The objective function to be maximized (or minimized) while
//! seeking for plans.
class Metric {
public:
	typedef const Metric* cptr;

	Metric(MetricType type, const Term* expr_)
		: _type(type), _expression(expr_), _valid(true) {
		if (!isWellFormed()) throw std::runtime_error("Ill-formed effect");
	}

    Metric( const Metric& other );

	virtual ~Metric() {
		delete _expression;
	}

	//! Checks that the metric expression is a well formed formula
	bool isWellFormed() const;

    MetricType  optimization() const { return _type; }

	//! Applies the effect to the given state and returns the resulting value
	object_id apply(const State& state) const;
	float evaluate(State& state) const { return fs0::value<float>(apply(state));}

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const Metric& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;

	//! Accessors for the left-hand side and right-hand side of the effect
	const Term* expression() const { return _expression; }

	void 	markAsInvalid() { _valid = false; }
	bool 	isValid() const { return _valid; }

protected:
	MetricType 	      _type;
	const Term* 	  _expression;
	bool 	          _valid;
};

} } } // namespaces
