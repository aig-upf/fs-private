
#pragma once

#include <search/algorithms/ehc.hxx>
#include <aptk2/search/algorithms/best_first_search.hxx>

#include <search/drivers/registry.hxx>
#include <state.hxx>
#include <problem.hxx>

namespace fs0 { namespace drivers {

//! A combined search strategy that first applies a given Enhanced Hill-Climbing and then, if the goal was not found,
//! a standard GBFS.
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
		bool result = false;
		
		if (_ehc) {
			result = _ehc->search(state, solution);
			expanded += _ehc->expanded;
			generated += _ehc->generated;
			if (result) {
				LPT_INFO("cout", "Solution found in EHC phase");
				return true;
			}
			
			// If EHC found no solution, then we switch to a standard GBFS.
			LPT_INFO("cout", "No solution found in EHC phase, switching to a GBFS");
			solution.clear();
		}

		result = _gbfs->search(state, solution);
		expanded += _gbfs->expanded;
		generated += _gbfs->generated;
		return result;
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
