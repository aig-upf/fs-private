

#include "iw_run_config.hxx"
#include <fs/core/utils/config.hxx>


namespace fs0 { namespace bfws {


        IWRunConfig::IWRunConfig(bool complete, bool mark_negative, unsigned max_width, const fs0::Config& global_config) :
                _complete(complete),
                _mark_negative(mark_negative),
                _max_width(max_width),
                _global_config(global_config),
                _goal_directed(global_config.getOption<bool>("goal_directed", false)),
                _force_adaptive_run(global_config.getOption<bool>("sim.hybrid", false)),
                _force_R_all(global_config.getOption<bool>("sim.r_all", false)),
                _r_g_prime(global_config.getOption<bool>("sim.r_g_prime", false)),
                _gr_actions_cutoff(global_config.getOption<unsigned>("sim.act_cutoff", std::numeric_limits<unsigned>::max())),
                _enforce_state_constraints(global_config.getOption<bool>("sim.enforce_state_constraints", false)),
                _R_file(global_config.getOption<std::string>("sim.from_file", "")),
                _filter_R_set(global_config.getOption<bool>("sim.filter", false)),
                _log_search(global_config.getOption<bool>("sim.log", false))
        {}

} } // namespaces
