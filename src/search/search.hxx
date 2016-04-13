
#pragma once

#include <fs_types.hxx>

namespace fs0 { class Problem; class Config; class LiftedActionID; }

namespace fs0 { namespace drivers {

class SearchUtils {
public:
	
	template <typename StateModelT, typename SearchAlgorithmT>
	static void do_search(SearchAlgorithmT& engine, const StateModelT& model, const std::string& out_dir, float start_time);

	//! Instantiate 
	static void instantiate_seach_engine_and_run(Problem& problem, const Config& config, const std::string& out_dir, float start_time);
	
	//! Print out some information about the characteristics of the problem
	static void report_stats(const Problem& problem);
};

} } // namespaces
