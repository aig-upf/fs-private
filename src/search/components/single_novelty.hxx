
#pragma once

#include <lapkt/tools/logging.hxx>
#include <lapkt/components/open_lists.hxx>

#include <fs_types.hxx>
#include <heuristics/novelty/fs0_novelty_evaluator.hxx>
#include <heuristics/novelty/novelty_features_configuration.hxx>


namespace fs0 { namespace drivers {

//! A novelty evaluator object to be used as an open list acceptor.
//! It accepts a new search node iff its novelty less than or equal to the max novelty bound
template <typename StateModelT,
          typename SearchNode,
          typename NoveltyEvaluatorT = GenericNoveltyEvaluator>
class NoveltyBasedAcceptor : public lapkt::QueueAcceptorI<SearchNode> {
protected:
	unsigned _max_novelty;	
	
	//! A single novelty evaluator will be in charge of evaluating all nodes
	NoveltyEvaluatorT _novelty_evaluator;

public:
	NoveltyBasedAcceptor(const StateModelT& model, unsigned max_novelty, const NoveltyFeaturesConfiguration& feature_configuration)
		: _max_novelty(max_novelty), _novelty_evaluator(model.getTask(), max_novelty, feature_configuration)
	{}
	
	inline unsigned novelty_bound() { return _max_novelty; }
	
	~NoveltyBasedAcceptor() {
		for ( unsigned k = 1; k <= novelty_bound(); k++ ) {
			LPT_INFO("heuristic", "# novelty(s)=" << k << " : " << _novelty_evaluator.get_num_states(k));
		}
	}

	NoveltyEvaluatorT& evaluator(const State& state) { return _novelty_evaluator; }
	
	inline unsigned novelty(const State& state) { return evaluator(state).evaluate(state); }
	
	//! Returns false iff we want to prune this node during the search
	virtual bool accept(SearchNode& n) {
		return novelty(n.state) <= novelty_bound();
	}	
};

} } // namespaces
