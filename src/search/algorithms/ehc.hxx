
#pragma once

#include <search/nodes/heuristic_search_node.hxx>
#include <search/nodes/blind_search_node.hxx>
#include <ground_state_model.hxx>
#include <state.hxx>

#include <aptk2/search/interfaces/search_algorithm.hxx>
#include <aptk2/search/algorithms/breadth_first_search.hxx>


namespace fs0 { namespace drivers {

//!
template <typename HeuristicT>
class EHCStateModel : public GroundStateModel {
public:
	EHCStateModel(const Problem& problem, HeuristicT& heuristic, float bound) :
		GroundStateModel(problem),
		_bound(bound),
		_heuristic(heuristic),
		_last_improving_state(0, {})
	{}
	~EHCStateModel() = default;
	
	EHCStateModel(const EHCStateModel&) = default;
	EHCStateModel& operator=(const EHCStateModel&) = default;
	EHCStateModel(EHCStateModel&&) = default;
	EHCStateModel& operator=(EHCStateModel&&) = default;

	//! Returns true if state is a goal state
	bool goal(const State& state) const override {
		_last_h = _heuristic.evaluate(state);
		if (_last_h < _bound) {
			_last_improving_state = state; // Perform a copy
			return true;
		}
		return false;
	}
	
	
	float get_last_heuristic_value() const { return _last_h; }
	
	State get_last_improving_state() const { return _last_improving_state; }
	
protected:
	const float _bound;
	
	HeuristicT& _heuristic;
	
	mutable float _last_h;
	
	mutable State _last_improving_state;
};


//! Enhanced Hill-Climbing with a generic heuristic evaluator.
template <typename HeuristicT>
class EHCSearch {
public:
	//! 
// 	typedef HeuristicSearchNode<State, GroundAction> SearchNode;
	typedef BlindSearchNode<State> SearchNode;
	
	//! EHC uses a breadth-first search as a base.
	typedef aptk::StlBreadthFirstSearch<SearchNode, EHCStateModel<HeuristicT>> BreadthFirstAlgorithm;
	
	~EHCSearch() = default;
	EHCSearch(const EHCSearch&) = default;
	EHCSearch(EHCSearch&&) = default;
	EHCSearch& operator=(const EHCSearch&) = default;
	EHCSearch& operator=(EHCSearch&&) = default;
	
	EHCSearch(const Problem& problem, HeuristicT&& heuristic) :
		_problem(problem), _heuristic(std::move(heuristic))
	{}
	
	bool search(const State& state, std::vector<unsigned>& solution) {
		assert(solution.size()==0);
		LPT_INFO("cout", "Starting EHC search on state " << state);
		
		State s = state; // Copy the state
		float h = _heuristic.evaluate(s);
		
		while(h > 0) {
			std::vector<unsigned> partial;
			
			// Perform breadth-first search until a state with smaller heuristic value is found
			EHCStateModel<HeuristicT> ehc_model(_problem, _heuristic, h); // Set up an EHC model with h as the heuristic bound
			BreadthFirstAlgorithm bfs(ehc_model); //! A standard breadth-first search
			
			if (!bfs.search(s, partial)) { // EHC fails
				LPT_INFO("cout", "EHC's breadth-first search unable to find a state with lower heuristic h(s) < " << h);
				return false;
			}
			
			solution.insert(solution.end(), partial.begin(), partial.end());
			
			s =  ehc_model.get_last_improving_state();
			h = ehc_model.get_last_heuristic_value();
		}

		return true;
	}

protected:
	//!
	const Problem& _problem;
	
	//!
	HeuristicT _heuristic;
};

} } // namespaces
