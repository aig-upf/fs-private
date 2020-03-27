
#pragma once

#include <tuple>
#include <vector>


namespace fs0::bfws {

class BFWSStats {
public:
    BFWSStats();

    void reset_generations() { _generated=0; }

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
    void sim_add_expanded_nodes(unsigned number) { _sim_expanded_nodes += number; }
    void sim_add_generated_nodes(unsigned number) { _sim_generated_nodes += number; }
    void sim_add_time(float time) { _sim_time += time; }

    void monot_pruned() { ++_monot_pruned; }

    void sim_table_created(unsigned k) {
        if (k >= _sim_wtables.size()) _sim_wtables.resize(k+1);
        ++_sim_wtables[k];
    }

    void search_table_created(unsigned k) {
        if (k >= _search_wtables.size()) _search_wtables.resize(k+1);
        ++_search_wtables[k];
    }

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

    void set_initial_reward(float r) { _initial_reward = r; }
    void reward(float r) {
        _max_reward = std::max(r, _max_reward);
    }

    float initial_reward() const { return _initial_reward; }
    float max_reward() const { return _max_reward; }

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

    void r_type(unsigned type) {
        _r_type = type;
    }

    using DataPointT = std::tuple<std::string, std::string, std::string>;
    std::vector<DataPointT> dump() const;


    void log_start_of_search(double initial_search_time) {
        _initial_search_time = initial_search_time;
    }

    double initial_search_time() const {
        if (_initial_search_time < 0) {
            throw std::runtime_error("Attempted to access initial_search_time without having invoked"
                                     " method 'log_start_of_search'");
        }
        return _initial_search_time;
    }

protected:

    static std::string _if_computed(unsigned val);
    static std::string _avg(unsigned long val, unsigned den);

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

    unsigned _r_type;

    unsigned long _num_wg1_nodes; // The number of nodes with w_{#g} = 1 that have been processed.
    unsigned long _num_wgr1_nodes; // The number of nodes with w_{#g,#r} = 1 (and w_{#g} > 1) that have been processed.
    unsigned long _num_wg1_5_nodes;
    unsigned long _num_wgr2_nodes; // The number of nodes with w_{#g,#r} = 2 (and w_{#g} > 1) that have been processed.
    unsigned long _num_wgr_gt2_nodes; // The number of nodes with w_{#g,#r} > 2 (and w_{#g} > 1) that have been processed.
    unsigned long _num_expanded_g_decrease; // The number of nodes with a decrease in #g that are expanded
    unsigned long _num_generated_g_decrease; // The number of nodes with a decrease in #g that are expanded

    unsigned long _monot_pruned;

    unsigned _reused_simulation_nodes;

    unsigned long _sim_expanded_nodes;
    unsigned long _sim_generated_nodes;
    float _sim_time;

    //! _sim_wtables[w] contains the number of width-w novelty tables created during simulation
    std::vector<unsigned> _sim_wtables;
    std::vector<unsigned> _search_wtables;
    float   _initial_reward = 0.0f;
    float   _max_reward = -std::numeric_limits<float>::max();

    double _initial_search_time;
};

} // namespaces
