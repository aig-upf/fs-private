
#pragma once

#include <search/algorithms/ehc.hxx>
#include <aptk2/search/algorithms/best_first_search.hxx>

#include <search/drivers/registry.hxx>
#include <state.hxx>
#include <problem.hxx>

namespace fs0 { namespace drivers {

//! Enhanced Hill-Climbing with a generic heuristic evaluator.
template <typename HeuristicT>
class EHCThenGBFSSearch {
public:
	~EHCThenGBFSSearch() = default;
	EHCThenGBFSSearch(const EHCThenGBFSSearch&) = default;
	EHCThenGBFSSearch(EHCThenGBFSSearch&&) = default;
	EHCThenGBFSSearch& operator=(const EHCThenGBFSSearch&) = default;
	EHCThenGBFSSearch& operator=(EHCThenGBFSSearch&&) = default;
	
	EHCThenGBFSSearch(const Problem& problem, FSGroundSearchAlgorithm* gbfs, EHCSearch<HeuristicT>* ehc) :
		_problem(problem), _gbfs(gbfs), _ehc(ehc)
	{}
	
	bool search(const State& state, std::vector<unsigned>& solution) {
		assert(solution.size()==0);
		
		if (_ehc) {
			if (_ehc->search(state, solution)) {
				LPT_INFO("cout", "Solution found in EHC phase");
				return true;
			}
			
			// If EHC found no solution, then we switch to a standard GBFS.
			LPT_INFO("cout", "No solution found in EHC phase, switching to a GBFS");
			solution.clear();
		}

		return _gbfs->search(state, solution);
	}
	
	//! Convenience method
	bool solve_model(std::vector<unsigned>& solution) { return search( _problem.getInitialState(), solution ); }
	
	unsigned long expanded = 0;
	unsigned long generated = 0;

	
protected:
	
	//!
	const Problem& _problem;
	
	//!
	std::unique_ptr<FSGroundSearchAlgorithm> _gbfs;
	
	//!
	std::unique_ptr<EHCSearch<HeuristicT>> _ehc;
};

} } // namespaces
