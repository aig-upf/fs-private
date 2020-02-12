
#pragma once

#include <string>

namespace fs0 { class Config; }

namespace fs0::bfws {

struct IWRunConfig {
    //! Whether to perform a complete run or a partial one, i.e. up until (independent) satisfaction of all goal atoms.
    bool _complete;

    //! The maximum levels of novelty to be considered
    unsigned _max_width;

    //!
    const fs0::Config& _global_config;

    //! Whether to extract goal-informed relevant sets R
    bool _goal_directed;

    //!
    bool _force_R_all;

    //!
    bool _r_g_prime;

    //!
    unsigned _gr_actions_cutoff;

    //! Enforce state constraints
    bool _enforce_state_constraints;

    //! Log search
    bool _log_search;

    //! Use an "action achiever" novelty evaluator type
    bool _use_achiever_evaluator;

    IWRunConfig(bool complete, unsigned max_width, const fs0::Config& global_config);
};

} // namespaces