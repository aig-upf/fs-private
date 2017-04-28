
#pragma once

#include <string>
#include <tuple>
#include <vector>
#include <limits>
#include <sstream>
#include <ios>
#include <iomanip>

namespace fs0 { class Config; class Problem; }

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
	const bool complete_simulation;
	
	enum class NoveltyEvaluatorType {Adaptive, Generic};
	NoveltyEvaluatorType evaluator_t;
	
	enum class RelevantSetType {None, Sim};
	RelevantSetType relevant_set_type;
};

class BFWSStats {
public:
	BFWSStats() : _expanded(0), _generated(0), _evaluated(0),
		_initial_reachable_subgoals(std::numeric_limits<unsigned>::max()),
		_max_reachable_subgoals(0),
		_sum_reachable_subgoals(0),
		_initial_relevant_atoms(std::numeric_limits<unsigned>::max()),
		_max_relevant_atoms(0),
		_sum_relevant_atoms(0)
	{}
	
	void expansion() { ++_expanded; }
	void generation() { ++_generated; }
	void evaluation() { ++_evaluated; }
	
	void wg1_node() { ++_num_wg1_nodes; }
	void wgr1_node() { ++_num_wgr1_nodes; }
	void wg1_5_node() { ++_num_wg1_5_nodes; }
	void wgr2_node() { ++_num_wgr2_nodes; }
	void wgr_gt2_node() { ++_num_wgr_gt2_nodes; }

	void simulation() { ++_simulations; }
	void simulation_node_reused() { ++_reused_simulation_nodes; }
	void sim_add_reached_subgoals(unsigned number) { _sim_reached_subgoals += number; }
	void sim_add_expanded_nodes(unsigned number) { _sim_expanded_nodes += number; }
	void sim_add_generated_nodes(unsigned number) { _sim_generated_nodes += number; }
	void sim_add_time(float time) { _sim_time += time; }
	
	void expansion_g_decrease() { ++_num_expanded_g_decrease; }
	void generation_g_decrease() { ++_num_generated_g_decrease; }

	unsigned long num_wg1_nodes() const { return _num_wg1_nodes; }
	unsigned long num_wgr1_nodes() const { return _num_wgr1_nodes; }
	unsigned long num_wg1_5_nodes() const { return _num_wg1_5_nodes; }
	unsigned long num_wgr2_nodes() const { return _num_wgr2_nodes; }
	unsigned long num_wgr_gt2_nodes() const { return _num_wgr_gt2_nodes; }
	
	unsigned long expanded() const { return _expanded; }
	unsigned long generated() const { return _generated; }
	unsigned long evaluated() const { return _evaluated; }
	unsigned long simulated() const { return _simulations; }
	
	
	void set_num_subgoals(unsigned num) { _num_subgoals = num; }
	void set_initial_reachable_subgoals(unsigned num) { _initial_reachable_subgoals = num; }
	void set_initial_relevant_atoms(unsigned num) { _initial_relevant_atoms = num; }
	void reachable_subgoals(unsigned num) {
		_max_reachable_subgoals = std::max(num, _max_reachable_subgoals);
		_sum_reachable_subgoals += num;
	}
	void relevant_atoms(unsigned num) {
		_max_relevant_atoms = std::max(num, _max_relevant_atoms);
		_sum_relevant_atoms += num;
	}
	
	static std::string _if_computed(unsigned val) {
		return val < std::numeric_limits<unsigned>::max() ?  std::to_string(val) : "N/A";
	}
	
	static std::string _avg(unsigned val, unsigned den) {
		if (den == 0) return "N/A";
		std::stringstream ss;
		ss << std::fixed << std::setprecision(2) << val / (float) den;
		return ss.str();
	}
	
	using DataPointT = std::tuple<std::string, std::string, std::string>;
	std::vector<DataPointT> dump() const {
		return {
			std::make_tuple("expanded", "Expansions", std::to_string(expanded())),
			std::make_tuple("generated", "Generations", std::to_string(generated())),
			std::make_tuple("evaluated", "Evaluations", std::to_string(evaluated())),

			std::make_tuple("_num_wg1_nodes", "w_{#g}(n)=1", std::to_string(_num_wg1_nodes)),
			std::make_tuple("_num_wgr1_nodes", "w_{#g,#r}(n)=1", std::to_string(_num_wgr1_nodes)),
			std::make_tuple("_num_wg1_5_nodes", "w_{#g}(n)=1.5", std::to_string(_num_wg1_5_nodes)),
			std::make_tuple("_num_wgr2_nodes", "w_{#g,#r}(n)=2", std::to_string(_num_wgr2_nodes)),
			std::make_tuple("_num_wgr_gt2_nodes", "w_{#g,#r}(n)>2", std::to_string(_num_wgr_gt2_nodes)),
			
			std::make_tuple("_num_expanded_g_decrease", "Expansions with #g decrease", std::to_string(_num_expanded_g_decrease)),
			std::make_tuple("_num_generated_g_decrease", "Generations with #g decrease", std::to_string(_num_generated_g_decrease)),
			
			std::make_tuple("num_subgoals", "Total number of conjunctive subgoals", std::to_string(_num_subgoals)),
			
			std::make_tuple("sim_num_simulations", "Total number of simulations", std::to_string(simulated())),
			
			std::make_tuple("sim_time", "Total simulation time", std::to_string(_sim_time)),
			std::make_tuple("sim_expanded_nodes", "Total nodes expanded during simulations", std::to_string(_sim_expanded_nodes)),
			std::make_tuple("sim_generated_nodes", "Total nodes generated during simulation", std::to_string(_sim_generated_nodes)),
			
			std::make_tuple("sim_avg_time", "Avg. simulation time", _avg(_sim_time, _simulations)),
			std::make_tuple("sim_avg_expanded_nodes", "Avg. nodes expanded during simulations", _avg(_sim_expanded_nodes, _simulations)),
			std::make_tuple("sim_avg_generated_nodes", "Avg. nodes generated during simulation", _avg(_sim_generated_nodes, _simulations)),
			
			std::make_tuple("sim_avg_reached_subgoals", "Avg. number of subgoals reached during simulations", std::to_string(_sim_reached_subgoals)),
			
			std::make_tuple("reused_simulation_nodes", "Simulation nodes reused in the search", std::to_string(_reused_simulation_nodes)),
			
			std::make_tuple("sim_reachable_0", "Reachable subgoals in initial state", _if_computed(_initial_reachable_subgoals)),
			std::make_tuple("sim_reachable_max", "Max. # reachable subgoals in any simulation", std::to_string(_max_reachable_subgoals)),
			std::make_tuple("sim_reachable_avg", "Avg. # reachable subgoals in any simulation", _avg(_sum_reachable_subgoals, _simulations)),
			std::make_tuple("sim_relevant_atoms_0", "|R|_0", _if_computed(_initial_relevant_atoms)),
			std::make_tuple("sim_relevant_atoms_max", "|R|_max", std::to_string(_max_relevant_atoms)),
			std::make_tuple("sim_relevant_atoms_avg", "|R|_avg", _avg(_sum_relevant_atoms, _simulations))
		};
	}
	
protected:
	unsigned long _expanded;
	unsigned long _generated;
	unsigned long _evaluated;
	unsigned long _simulations;
	unsigned int _initial_reachable_subgoals; // The number of subgoals that are reachable on the initial simulation
	unsigned int _max_reachable_subgoals; // The max. number of subgoals that are reachable in any simulation
	unsigned int _sum_reachable_subgoals; // The sum of # reached subgoals, to obtain an average
	unsigned int _initial_relevant_atoms; // The size of |R| on the initial state
	unsigned int _max_relevant_atoms;
	unsigned int _sum_relevant_atoms;
	
	unsigned long _num_wg1_nodes; // The number of nodes with w_{#g} = 1 that have been processed.
	unsigned long _num_wgr1_nodes; // The number of nodes with w_{#g,#r} = 1 (and w_{#g} > 1) that have been processed.
	unsigned long _num_wg1_5_nodes;
	unsigned long _num_wgr2_nodes; // The number of nodes with w_{#g,#r} = 2 (and w_{#g} > 1) that have been processed.
	unsigned long _num_wgr_gt2_nodes; // The number of nodes with w_{#g,#r} > 2 (and w_{#g} > 1) that have been processed.
	unsigned long _num_expanded_g_decrease; // The number of nodes with a decrease in #g that are expanded
	unsigned long _num_generated_g_decrease; // The number of nodes with a decrease in #g that are expanded
	
	unsigned _reused_simulation_nodes;
	unsigned _sim_reached_subgoals;
	
	unsigned long _sim_expanded_nodes;
	unsigned long _sim_generated_nodes;
	float _sim_time;
	
	unsigned _num_subgoals;

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

//! A helper to create a novelty evaluator of the appropriate type
template <typename NoveltyEvaluatorT>
NoveltyEvaluatorT* create_novelty_evaluator(const Problem& problem, SBFWSConfig::NoveltyEvaluatorType evaluator_t, unsigned max_width, bool persistent = false);


} } // namespaces

