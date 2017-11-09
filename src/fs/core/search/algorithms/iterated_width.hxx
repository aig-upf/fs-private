
#pragma once

#include <functional>

#include <fs/core/search/nodes/blind_node.hxx>
#include <lapkt/search/components/open_lists.hxx>
#include <lapkt/algorithms/breadth_first_search.hxx>

#include <lapkt/novelty/novelty_based_acceptor.hxx>

#include <fs/core/search/drivers/registry.hxx>
#include <fs/core/search/drivers/setups.hxx>

#include <fs/core/search/events.hxx>
#include <fs/core/search/stats.hxx>


namespace fs0 { class SearchStats; }

namespace fs0 { namespace drivers {

//! The original IW algorithm, adapted to FStrips
template <typename StateModelT, typename FeatureSetT, typename NoveltyEvaluatorT>
class FS0IWAlgorithm {
public:
	using ActionT = typename StateModelT::ActionType;

	using PlanT = std::vector<typename StateModelT::ActionType::IdType>;

	//! IW uses a simple blind-search node
	using NodeT = lapkt::BlindSearchNode<State, ActionT>;

	//! IW uses an unsorted queue with a NoveltyEvaluator acceptor
	using OpenList = lapkt::SearchableQueue<NodeT>;

	//! The base algorithm for IW is a simple Breadth-First Search
	using BaseAlgorithm = lapkt::StlBreadthFirstSearch<NodeT, StateModelT, OpenList>;

	using NoveltyEvaluatorPT = std::unique_ptr<NoveltyEvaluatorT>;

	FS0IWAlgorithm(const StateModelT& model, unsigned initial_max_width, unsigned final_max_width, FeatureSetT&& featureset, NoveltyEvaluatorT* evaluator_prototype, SearchStats& stats)
		: _model(model), _featureset(std::move(featureset)), _evaluator_prototype(evaluator_prototype), _algorithm(nullptr) , _current_max_width(initial_max_width), _final_max_width(final_max_width), _stats(stats)
	{
		EventUtils::setup_stats_observer<NodeT>(_stats, _handlers);
		setup_base_algorithm(_current_max_width);
	}

	~FS0IWAlgorithm() = default;

	FS0IWAlgorithm(const FS0IWAlgorithm&) = default;
	FS0IWAlgorithm(FS0IWAlgorithm&&) = default;
	FS0IWAlgorithm& operator=(const FS0IWAlgorithm&) = default;
	FS0IWAlgorithm& operator=(FS0IWAlgorithm&&) = default;

	bool search(const State& state, PlanT& solution) {
		while(_current_max_width <= _final_max_width) {
			LPT_INFO("search", "IW: Starting search with novelty bound of " << _current_max_width);
			if(_algorithm->search(state, solution)) return true;
			++_current_max_width;
			setup_base_algorithm(_current_max_width);
			solution.clear();
		}
		return false;
	}

	void setup_base_algorithm(unsigned max_width) {
		//! IW uses a single novelty component as the open list evaluator
		using NoveltyAcceptor = lapkt::novelty::NoveltyBasedAcceptor<NodeT, FeatureSetT, NoveltyEvaluatorT>;
		NoveltyAcceptor* evaluator = new NoveltyAcceptor(_featureset, _evaluator_prototype->clone());

		_algorithm = std::unique_ptr<BaseAlgorithm>(new BaseAlgorithm(_model, OpenList(evaluator)));
		lapkt::events::subscribe(*_algorithm, _handlers);
	}

	//! Convenience method
	bool solve_model(PlanT& solution) { return search( _model.getTask().getInitialState(), solution ); }


protected:

	//!
	const StateModelT& _model;

	//!
	const FeatureSetT _featureset;

	//! We keep a "base" novelty evaluator with the appropriate features
	NoveltyEvaluatorPT _evaluator_prototype;

	//!
	std::unique_ptr<BaseAlgorithm> _algorithm;

	//!
	unsigned _current_max_width;

	//!
	unsigned _final_max_width;

	//!
	std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;

	//! A reference to the stats object managed by the parent.
	SearchStats& _stats;
};



} } // namespaces
