
#include <limits>
#include <ios>
#include <iomanip>
#include <sstream>

#include <fs/core/search/drivers/sbfws/stats.hxx>

namespace fs0 { namespace bfws {

BFWSStats::BFWSStats() : _expanded(0), _generated(0), _evaluated(0),
	_initial_reachable_subgoals(std::numeric_limits<unsigned>::max()),
	_max_reachable_subgoals(0),
	_sum_reachable_subgoals(0),
	_initial_relevant_atoms(std::numeric_limits<unsigned>::max()),
	_max_relevant_atoms(0),
	_sum_relevant_atoms(0)
{}

std::string
BFWSStats::_if_computed(unsigned val) {
	return val < std::numeric_limits<unsigned>::max() ?  std::to_string(val) : "N/A";
}

std::string 
BFWSStats::_avg(unsigned val, unsigned den) {
	if (den == 0) return "N/A";
	std::stringstream ss;
	ss << std::fixed << std::setprecision(2) << val / (float) den;
	return ss.str();
}
	
std::vector<BFWSStats::DataPointT> BFWSStats::dump() const {
	std::vector<BFWSStats::DataPointT> data = {
		std::make_tuple("expanded", "Expansions", std::to_string(expanded())),
		std::make_tuple("generated", "Generations", std::to_string(generated())),
		std::make_tuple("evaluated", "Evaluations", std::to_string(evaluated())),

		std::make_tuple("_num_wg1_nodes", "w_{#g}(n)=1", std::to_string(_num_wg1_nodes)),
		std::make_tuple("_num_wgr1_nodes", "w_{#g,#r}(n)=1", std::to_string(_num_wgr1_nodes)),
// 		std::make_tuple("_num_wg1_5_nodes", "w_{#g}(n)=1.5", std::to_string(_num_wg1_5_nodes)),
		std::make_tuple("_num_wgr2_nodes", "w_{#g,#r}(n)=2", std::to_string(_num_wgr2_nodes)),
		std::make_tuple("_num_wgr_gt2_nodes", "w_{#g,#r}(n)>2", std::to_string(_num_wgr_gt2_nodes)),
		
		std::make_tuple("_num_expanded_g_decrease", "Expansions with #g decrease", std::to_string(_num_expanded_g_decrease)),
		std::make_tuple("_num_generated_g_decrease", "Generations with #g decrease", std::to_string(_num_generated_g_decrease)),
		
// 		std::make_tuple("num_subgoals", "Total number of conjunctive subgoals", std::to_string(_num_subgoals)),
		
		std::make_tuple("sim_num_simulations", "Total number of simulations", std::to_string(simulated())),
		
		std::make_tuple("sim_time", "Total simulation time", std::to_string(_sim_time)),
		std::make_tuple("sim_expanded_nodes", "Total nodes expanded during simulations", std::to_string(_sim_expanded_nodes)),
		std::make_tuple("sim_generated_nodes", "Total nodes generated during simulation", std::to_string(_sim_generated_nodes)),
		
		std::make_tuple("sim_avg_time", "Avg. simulation time", _avg(_sim_time, _simulations)),
		std::make_tuple("sim_avg_expanded_nodes", "Avg. nodes expanded during simulations", _avg(_sim_expanded_nodes, _simulations)),
		std::make_tuple("sim_avg_generated_nodes", "Avg. nodes generated during simulation", _avg(_sim_generated_nodes, _simulations)),
		
		std::make_tuple("sim_avg_reached_subgoals", "Avg. number of subgoals reached during simulations", _avg(_sum_reachable_subgoals, _simulations)),
		
		std::make_tuple("reused_simulation_nodes", "Simulation nodes reused in the search", std::to_string(_reused_simulation_nodes)),
		
		std::make_tuple("r_type", "Type of R set", std::to_string(_r_type)),
		
		std::make_tuple("sim_reachable_0", "Reachable subgoals in initial state", _if_computed(_initial_reachable_subgoals)),
		std::make_tuple("sim_reachable_max", "Max. # reachable subgoals in any simulation", std::to_string(_max_reachable_subgoals)),
		std::make_tuple("sim_reachable_avg", "Avg. # reachable subgoals in any simulation", _avg(_sum_reachable_subgoals, _simulations)),
		std::make_tuple("sim_relevant_atoms_0", "|R|_0", _if_computed(_initial_relevant_atoms)),
		std::make_tuple("sim_relevant_atoms_max", "|R|_max", std::to_string(_max_relevant_atoms)),
		std::make_tuple("sim_relevant_atoms_avg", "|R|_avg", _avg(_sum_relevant_atoms, _simulations)),
	};
	
	for (unsigned k = 1; k < _sim_wtables.size(); ++k) {
		std::string kstr = std::to_string(k);
		data.push_back(std::make_tuple("sim_w" + kstr + "_tables", "Number of width-" + kstr + " tables created during simulation", std::to_string(_sim_wtables[k])));
	}
	
	for (unsigned k = 1; k < _search_wtables.size(); ++k) {
		std::string kstr = std::to_string(k);
		data.push_back(std::make_tuple("search_w" + kstr + "_tables", "Number of width-" + kstr + " tables created during search", std::to_string(_search_wtables[k])));
	}	
	
	
	return data;
}
	
} } // namespaces
