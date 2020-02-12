

#include <fs/core/search/drivers/sbfws/iw_run_config.hxx>
#include <fs/core/utils/config.hxx>


namespace fs0::bfws {


IWRunConfig::IWRunConfig(unsigned max_width, const fs0::Config& global_config) :
        _max_width(max_width),
        _global_config(global_config),
        _goal_directed(global_config.getOption<bool>("goal_directed", false)),
        _force_R_all(global_config.getOption<bool>("sim.r_all", false)),
        _r_g_prime(global_config.getOption<bool>("sim.r_g_prime", false)),
        _gr_actions_cutoff(global_config.getOption<unsigned>("sim.act_cutoff", std::numeric_limits<unsigned>::max())),
        _enforce_state_constraints(global_config.getOption<bool>("sim.enforce_state_constraints", false)),
        _log_search(global_config.getOption<bool>("sim.log", false)),
        _use_achiever_evaluator(global_config.getOption<bool>("sim.achiever_novelty", false))
{}

} // namespaces
