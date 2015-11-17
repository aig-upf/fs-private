
#pragma once

#include <fs0_types.hxx>

namespace fs0 { class Problem; class Config; class LiftedActionID; }

namespace fs0 { namespace engines {

class SearchUtils {
public:
	
	template <typename StateModelT, typename SearchAlgorithmT>
	static float do_search(SearchAlgorithmT& engine, const StateModelT& model, const std::string& out_dir);

	//! Instantiate 
	static void instantiate_seach_engine_and_run(const Problem& problem, const Config& config, int timeout, const std::string& out_dir);
	
	//! Print out some information about the characteristics of the problem
	static void report_stats(const Problem& problem);
};

} } // namespaces
