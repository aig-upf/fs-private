
#pragma once

#include <functional>

#include <search/nodes/blind_search_node.hxx>
#include <search/components/single_novelty.hxx>
#include <search/engines/registry.hxx>
#include <state_model.hxx>

#include <aptk2/search/algorithms/generic_search.hxx>
#include <aptk2/search/algorithms/breadth_first_search.hxx>
#include <aptk2/search/components/unsorted_open_list_impl.hxx>
#include <aptk2/search/components/closed_list_impl.hxx>
#include <aptk2/search/components/stl_unsorted_fifo_open_list.hxx>

namespace fs0 { namespace engines {

//! The original IW algorithm, adapted to FStrips
class FS0IWAlgorithm : public FS0SearchAlgorithm {
public:
	//! IW uses a simple blind-search node
	typedef BlindSearchNode<State> SearchNode;
	
	//! IW uses a single novelty component as the open list evaluator
	typedef SingleNoveltyComponent<SearchNode> SearchNoveltyEvaluator;
	
	//! IW uses an unsorted queue with a NoveltyEvaluator acceptor
	typedef aptk::StlUnsortedFIFO<SearchNode, SearchNoveltyEvaluator> OpenList;
	
	//! The base algorithm for IW is a simple Breadth-First Search
	typedef aptk::StlBreadthFirstSearch<SearchNode, FS0StateModel, OpenList> BaseAlgorithm;
	
	FS0IWAlgorithm(const FS0StateModel& model, unsigned initial_max_width, unsigned final_max_width, bool use_state_vars, bool use_goal, bool use_actions)
		: FS0SearchAlgorithm(model), _current_max_width(initial_max_width), _final_max_width(final_max_width), _use_state_vars(use_state_vars), _use_goal(use_goal), _use_actions(use_actions)
	{
	}
	
	virtual ~FS0IWAlgorithm() {
		if (_algorithm) delete _algorithm;
	}
	
	
	virtual bool search(const State& state, typename FS0SearchAlgorithm::Plan& solution) {
		while(_current_max_width <= _final_max_width) {
			if(_algorithm->search(state, solution)) return true;
			++_current_max_width;
			setup_brfs_algorithm(_current_max_width);
			solution.clear();
		}
		return false;
	}
	
	void setup_brfs_algorithm(unsigned max_width) {
		if (_algorithm) delete _algorithm;
		std::shared_ptr<SearchNoveltyEvaluator> evaluator = std::make_shared<SearchNoveltyEvaluator>(this->model, _current_max_width, _use_state_vars, _use_goal, _use_actions);
		_algorithm = new BaseAlgorithm(model, OpenList(evaluator));
	}
	
protected:
	
	//!
	SearchAlgorithm* _algorithm;
	
	//!
	unsigned _current_max_width;
	
	//!
	unsigned _final_max_width;

	//! Novelty evaluator configuration - TODO This should be encapsulated out of here...
	bool _use_state_vars;
	bool _use_goal;
	bool _use_actions;
};

} } // namespaces
