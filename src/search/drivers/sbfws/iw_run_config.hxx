
#pragma once

#include <string>

namespace fs0 { class Config; }

namespace fs0 { namespace bfws {

struct IWRunConfig {
    //! Whether to perform a complete run or a partial one, i.e. up until (independent) satisfaction of all goal atoms.
    bool _complete;

    //! Whether to take into account negative propositions or not
    bool _mark_negative;

    //! The maximum levels of novelty to be considered
    unsigned _max_width;

    //!
    const fs0::Config& _global_config;

    //! Whether to extract goal-informed relevant sets R
    bool _goal_directed;

    //!
    bool _force_adaptive_run;

    //!
    bool _force_R_all;

    //!
    bool _r_g_prime;

    //!
    unsigned _gr_actions_cutoff;

    //! Enforce state constraints
    bool _enforce_state_constraints;

    //! Load R set from file
    std::string _R_file;

    //! Goal Ball filtering
    bool _filter_R_set;

    //! Log search
    bool _log_search;

    IWRunConfig(bool complete, bool mark_negative, unsigned max_width, const fs0::Config& global_config);
};

} } // namespaces