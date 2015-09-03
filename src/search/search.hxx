
#pragma once

#include <fs0_types.hxx>
#include <search/engine.hxx>

namespace fs0 { class Problem; class Config; }

namespace fs0 { namespace engines {

class SearchUtils {
public:
	static bool check_plan(const std::vector<GroundAction::IdType>& plan);

	static float do_search(fs0::engines::FS0SearchAlgorithm& engine, const Problem& problem, const std::string& out_dir);

	//! Instantiate 
	static void instantiate_seach_engine_and_run(const Problem& problem, const Config& config, int timeout, const std::string& out_dir);
	
	//! Print out some information about the characteristics of the problem
	static void report_stats(const Problem& problem);
};

} } // namespaces
