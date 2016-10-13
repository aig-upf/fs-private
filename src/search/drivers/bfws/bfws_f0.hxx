
#pragma once

#include <memory>

#include <search/drivers/bfws/bfws_base.hxx>
#include <search/drivers/registry.hxx>
#include <search/nodes/bfws_node.hxx>
#include <search/components/unsat_goals_novelty.hxx>
#include <search/algorithms/aptk/events.hxx>
#include <search/algorithms/aptk/best_first_search.hxx>
#include <search/stats.hxx>
#include <heuristics/relaxed_plan/smart_rpg.hxx>
#include <aptk2/search/components/stl_unsorted_fifo_open_list.hxx>

namespace fs0 { class GroundStateModel; class Config; }

namespace fs0 { namespace drivers {


template <typename StateModelT, typename ActionT>
class BFWSSubdriverF0 {
public:
	using NodeT = BFWSNode<fs0::State, ActionT>;
	using HeuristicT = UnsatGoalsNoveltyComponent<StateModelT, NodeT>;
	using Engine = std::unique_ptr<lapkt::StlBestFirstSearch<NodeT, HeuristicT, StateModelT>>;

	//!
	Engine create(const Config& config, BFWSConfig& bfws_config, const NoveltyFeaturesConfiguration& feature_configuration, const StateModelT& model);
	
	const SearchStats& getStats() const { return _stats; }

protected:
	//!
	std::unique_ptr<HeuristicT> _heuristic;
	
	//!
	std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;
	
	//!
	SearchStats _stats;
};



template <typename HeuristicT>
class BFWSHeuristicEnsemble {
public:
	BFWSHeuristicEnsemble(const GroundStateModel& model, unsigned max_novelty, const NoveltyFeaturesConfiguration& feature_configuration, std::unique_ptr<HeuristicT>&& heuristic) :
		_problem(model.getTask()),
		_feature_configuration(feature_configuration),
		_max_novelty(max_novelty), 
		_novelty_evaluators(),
		_base_heuristic(std::move(heuristic))
	{}
	
	~BFWSHeuristicEnsemble() = default;
	
	inline unsigned max_novelty() { return _max_novelty; }

	long compute_heuristic(const State& state) { 
		long h = _base_heuristic->evaluate(state);
		LPT_DEBUG("heuristic" , std::endl << "Computed heuristic value of " << h <<  " for state: " << std::endl << state << std::endl << "****************************************");
		return h;
	}
	
	//! Compute the novelty of the state wrt all the states with the same heuristic value.
	unsigned novelty(const State& state, long h) {
		
		auto it = _novelty_evaluators.find(h);
		if (it == _novelty_evaluators.end()) {
			auto inserted = _novelty_evaluators.insert(std::make_pair(h, GenericNoveltyEvaluator(_problem, _max_novelty, _feature_configuration)));
			it = inserted.first;
		}
		return it->second.evaluate(state);
	}
	

protected:
	const Problem& _problem;
	
	const NoveltyFeaturesConfiguration& _feature_configuration;
	
	unsigned _max_novelty;

	//! We have one different novelty evaluators for each actual heuristic value that a node might have.
	std::unordered_map<long, GenericNoveltyEvaluator> _novelty_evaluators;
	
	std::unique_ptr<HeuristicT> _base_heuristic;
};




class BFWS1H1WNode {
public:
	using ptr_t = std::shared_ptr<BFWS1H1WNode>;
	
	State state;
	GroundAction::IdType action;
	
	ptr_t parent;

	//! Accummulated cost
	unsigned g;

	//! Novelty of the state
	unsigned novelty;
	
	//!
	long hff;
	
public:
	BFWS1H1WNode() = delete;
	~BFWS1H1WNode() = default;
	
	BFWS1H1WNode(const BFWS1H1WNode& other) = delete;
	BFWS1H1WNode(BFWS1H1WNode&& other) = delete;
	BFWS1H1WNode& operator=(const BFWS1H1WNode& rhs) = delete;
	BFWS1H1WNode& operator=(BFWS1H1WNode&& rhs) = delete;
	
	//! Constructor with full copying of the state (expensive)
	BFWS1H1WNode(const State& s) : BFWS1H1WNode(State(s), GroundAction::invalid_action_id, nullptr) {}

	//! Constructor with move of the state (cheaper)
	BFWS1H1WNode(State&& _state, GroundAction::IdType action_, ptr_t parent_) :
		state(std::move(_state)), action(action_), parent(parent_), g(parent ? parent->g+1 : 0), novelty(std::numeric_limits<unsigned>::max()), hff(std::numeric_limits<long>::max())
	{}

	bool has_parent() const { return parent != nullptr; }
	
	//! Required for the interface of some algorithms that might prioritise helpful actions.
	bool is_helpful() const { return false; }


	bool operator==( const BFWS1H1WNode& o ) const { return state == o.state; }

	std::size_t hash() const { return state.hash(); }

	//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const BFWS1H1WNode& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const { 
		return os << "{@ = " << this << ", s = " << state << ", g = " << g << ", novelty = " << novelty << ", h = " << hff << ", parent = " << parent << "}";
	}
	
	template <typename Heuristic>
	void evaluate_with( Heuristic& ensemble ) {
		hff = ensemble.compute_heuristic(state);
		novelty = ensemble.novelty(state, hff);
		if (novelty > ensemble.max_novelty()) {
			novelty = std::numeric_limits<unsigned>::max();
		}
	}
	
	
	void inherit_heuristic_estimate() {
		if (parent) {
			novelty = parent->novelty;
			hff = parent->hff;
		}
	}
	
	//! What to do when an 'other' node is found during the search while 'this' node is already in
	//! the open list
	void update_in_open_list(ptr_t other) {
		if (other->g < this->g) {
			this->g = other->g;
			this->action = other->action;
			this->parent = other->parent;
			this->novelty = other->novelty;
			this->hff = other->hff;
		}
	}

	bool dead_end() const { return hff == -1; }
};








template <typename NodeT,
          typename HeuristicT,
          typename NodeCompareT,
          typename HeuristicEnsembleT = BFWSHeuristicEnsemble<HeuristicT>,
          typename RawEngineT = lapkt::StlBestFirstSearch<NodeT, HeuristicEnsembleT, GroundStateModel, std::shared_ptr<NodeT>, NodeCompareT>,
          typename Engine = std::unique_ptr<RawEngineT>
>
class BFWS1H1WSubdriver {
public:
	using EngineT = Engine;

	//!
	EngineT create(const Config& config, BFWSConfig& bfws_config, const NoveltyFeaturesConfiguration& feature_configuration, const GroundStateModel& model);
	
	const SearchStats& getStats() const { return _stats; }

protected:
	//!
	std::unique_ptr<HeuristicEnsembleT> _heuristic;
	
	//!
	std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;
	
	//!
	SearchStats _stats;
};


// ATM we work with two possible node lexicographical orderings: either
// F1: <h_FF, w_<h_ff>> (i.e. prioritize nodes with lower heuristic value, then nodes with lower novelty ("novelty given heuristic value"))
// F2: <w_<h_ff>, h_FF> (i.e. prioritize nodes with lower novelty ("novelty given heuristic value"), then nodes with lower heuristic value)
// By default, both orderings also take into account the cost-to-go to break ties.
// (Underlying logic of the operator is: return true iff the second element should be popped before the first.)
template <typename NodeT,
          typename NodePtrT = std::shared_ptr<NodeT>
>
struct F1NodeComparer {
	bool operator()(const NodePtrT& n1, const NodePtrT& n2) const {
		if (n1->hff > n2->hff) return true;
		if (n1->hff < n2->hff) return false;
		if (n1->novelty > n2->novelty ) return true;
		if (n1->novelty < n2->novelty ) return false;
		return n1->g > n2->g;
	}
};

template <typename NodeT,
          typename NodePtrT = std::shared_ptr<NodeT>
>
struct F2NodeComparer {
	bool operator()(const NodePtrT& n1, const NodePtrT& n2) const {
		if (n1->novelty > n2->novelty ) return true;
		if (n1->novelty < n2->novelty ) return false;
		if (n1->hff > n2->hff) return true;
		if (n1->hff < n2->hff) return false;
		return n1->g > n2->g;
	}
};





// Name the common BFWS variations
using BFWS_F0_GROUND = BFWSSubdriverF0<GroundStateModel, GroundAction>;
using BFWS_F0_LIFTED = BFWSSubdriverF0<LiftedStateModel, LiftedActionID>;

using BFWS_F1 = BFWS1H1WSubdriver<BFWS1H1WNode, gecode::SmartRPG, F1NodeComparer<BFWS1H1WNode>>;
using BFWS_F2 = BFWS1H1WSubdriver<BFWS1H1WNode, gecode::SmartRPG, F2NodeComparer<BFWS1H1WNode>>;


} } // namespaces
