
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

	Metric(MetricType type, const Term* terminal_expr, const Term* stage_expr)
		: _type(type), _terminal_expr(terminal_expr), _stage_expr(stage_expr), _valid(true) {
		if (!isWellFormed()) throw std::runtime_error("Ill-formed effect");
	}

    Metric( const Metric& other );

	virtual ~Metric() {
		delete _terminal_expr;
		delete _stage_expr;
	}

	//! Checks that the metric expression is a well formed formula
	bool isWellFormed() const;

    MetricType  optimization() const { return _type; }

	//! Applies the effect to the given state and returns the resulting value
	object_id evaluate(const State& state, const Term* expr) const;
	float evaluate(const State& state) const { return fs0::value<float>(evaluate(state,_stage_expr));}
	float stage_cost(const State& state) const { return fs0::value<float>(evaluate(state,_stage_expr));}
	float terminal_cost(const State& state) const { return fs0::value<float>(evaluate(state,_terminal_expr));}

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const Metric& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;

	//! Accessors for the left-hand side and right-hand side of the effect
	const Term* terminal_expr() const { return _terminal_expr; }
	const Term* stage_expr() const { return _stage_expr; }

	void 	markAsInvalid() { _valid = false; }
	bool 	isValid() const { return _valid; }

protected:
	MetricType 	      _type;
	const Term* 	  _terminal_expr;
	const Term* 	  _stage_expr;
	bool 	          _valid;
};


} } } // namespaces
