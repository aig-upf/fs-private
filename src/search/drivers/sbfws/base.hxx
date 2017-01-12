
#pragma once

#include <string>
#include <tuple>
#include <vector>

namespace fs0 { class Config; }

namespace fs0 { namespace bfws {

struct SBFWSConfig {
	SBFWSConfig(const Config&);
	SBFWSConfig(const SBFWSConfig&) = default;
	SBFWSConfig& operator=(const SBFWSConfig&) = default;
	SBFWSConfig(SBFWSConfig&&) = default;
	SBFWSConfig& operator=(SBFWSConfig&&) = default;

	//! The maximum levels of width for search and simulation
	const unsigned search_width;
	const unsigned simulation_width;
	const bool mark_negative_propositions;
};

class BFWSStats {
public:
	BFWSStats() : _expanded(0), _generated(0), _evaluated(0) {}
	
	void expansion() { ++_expanded; }
	void generation() { ++_generated; }
	void evaluation() { ++_evaluated; }
	void simulation() { ++_simulations; }
	
	void wg1_node() { ++_num_wg1_nodes; }
	void wgr1_node() { ++_num_wgr1_nodes; }
	void wgr2_node() { ++_num_wgr2_nodes; }
	void wgr_gt2_node() { ++_num_wgr_gt2_nodes; }
	
	void expansion_g_decrease() { ++_num_expanded_g_decrease; }
	void generation_g_decrease() { ++_num_generated_g_decrease; }

	unsigned long num_wg1_nodes() const { return _num_wg1_nodes; }
	unsigned long num_wgr1_nodes() const { return _num_wgr1_nodes; }
	unsigned long num_wgr2_nodes() const { return _num_wgr2_nodes; }
	unsigned long num_wgr_gt2_nodes() const { return _num_wgr_gt2_nodes; }
	
	unsigned long expanded() const { return _expanded; }
	unsigned long generated() const { return _generated; }
	unsigned long evaluated() const { return _evaluated; }
	unsigned long simulated() const { return _simulations; }
	
	void set_initial_reachable_subgoals(unsigned num) { _initial_reachable_subgoals = num; }
	void set_initial_relevant_atoms(unsigned num) { _initial_relevant_atoms = num; }
	
	using DataPointT = std::tuple<std::string, std::string, std::string>;
	std::vector<DataPointT> dump() const {
		return {
			std::make_tuple("expanded", "Expansions", std::to_string(expanded())),
			std::make_tuple("generated", "Generations", std::to_string(generated())),
			std::make_tuple("evaluated", "Evaluations", std::to_string(evaluated())),

			std::make_tuple("_num_wg1_nodes", "w_{#g}(n)=1", std::to_string(_num_wg1_nodes)),
			std::make_tuple("_num_wgr1_nodes", "w_{#g,#r}(n)=1", std::to_string(_num_wgr1_nodes)),
			std::make_tuple("_num_wgr2_nodes", "w_{#g,#r}(n)=2", std::to_string(_num_wgr2_nodes)),
			std::make_tuple("_num_wgr_gt2_nodes", "w_{#g,#r}(n)>2", std::to_string(_num_wgr_gt2_nodes)),
			
			std::make_tuple("_num_expanded_g_decrease", "Expansions with #g decrease", std::to_string(_num_expanded_g_decrease)),
			std::make_tuple("_num_generated_g_decrease", "Generations with #g decrease", std::to_string(_num_generated_g_decrease)),

			std::make_tuple("simulations", "Simulations", std::to_string(simulated())),
			std::make_tuple("reachable_0", "Reachable subgoals in initial state", std::to_string(_initial_reachable_subgoals)),
			std::make_tuple("relevant_atoms_0", "|R|_0", std::to_string(_initial_relevant_atoms)),
		};
	}
	
protected:
	unsigned long _expanded;
	unsigned long _generated;
	unsigned long _evaluated;
	unsigned long _simulations;
	unsigned int _initial_reachable_subgoals; // The number of subgoals that are reachable on the initial simulation
	unsigned int _initial_relevant_atoms; // The size of |R| on the initial state
	
	unsigned long _num_wg1_nodes; // The number of nodes with w_{#g} = 1 that have been processed.
	unsigned long _num_wgr1_nodes; // The number of nodes with w_{#g,#r} = 1 (and w_{#g} > 1) that have been processed.
	unsigned long _num_wgr2_nodes; // The number of nodes with w_{#g,#r} = 2 (and w_{#g} > 1) that have been processed.
	unsigned long _num_wgr_gt2_nodes; // The number of nodes with w_{#g,#r} > 2 (and w_{#g} > 1) that have been processed.
	unsigned long _num_expanded_g_decrease; // The number of nodes with a decrease in #g that are expanded
	unsigned long _num_generated_g_decrease; // The number of nodes with a decrease in #g that are expanded
};

//! TODO - TOO HACKY?
inline unsigned _index(unsigned unachieved, unsigned relaxed_achieved) {
	return (unachieved<<16) | relaxed_achieved;
}

// Index the novelty tables by <#g, #r>
struct SBFWSNoveltyIndexer {
	unsigned operator()(unsigned unachieved, unsigned relaxed_achieved) const {
		return _index(unachieved, relaxed_achieved);
	}

	std::tuple<unsigned, unsigned> relevant(unsigned unachieved, unsigned relaxed_achieved) const {
		return std::make_tuple(unachieved, relaxed_achieved);
	}
};


} } // namespaces

