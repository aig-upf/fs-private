
#pragma once

#include <fs_types.hxx>
#include <state_model.hxx>
#include <utils/logging.hxx>
#include <heuristics/novelty/fs0_novelty_evaluator.hxx>

namespace fs0 { class Problem; class Config; }

namespace fs0 { namespace drivers {

//! A novelty evaluator object to be used as an open list acceptor.
//! It accepts a new search node iff its novelty less than or equal to the max novelty bound
template <typename SearchNode>
class BaseNoveltyComponent {
protected:
	unsigned _max_novelty;

public:

	BaseNoveltyComponent(unsigned max_novelty)
		: _max_novelty(max_novelty)
	{}

	virtual ~BaseNoveltyComponent() {}

	inline unsigned novelty_bound() { return _max_novelty; }
	
	//! This must be subclassed to return the novelty evaluator in charge of evaluating the given state
	//! (which might e.g. depend on the number of achieved goals in the state, etc.)
	virtual GenericNoveltyEvaluator& evaluator(const State& state) = 0;

	inline unsigned novelty(const State& state) { return evaluator(state).evaluate(state); }

	//! Returns false iff we want to prune this node during the search
	bool accept(const SearchNode& n) {
		return novelty(n.state) <= novelty_bound();
	}
};

} } // namespaces
