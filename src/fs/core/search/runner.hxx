
#pragma once

#include <fs/core/search/options.hxx>
#include <rapidjson/document.h>

namespace fs0 { class Problem; }

namespace fs0 { namespace drivers {
	
//! A basic runner script to bootstrap the problem and run the search engine
class Runner {
public:
	//! The type of the concrete instance generator function
	typedef std::function<Problem* (const rapidjson::Document&, const std::string&)> ProblemGeneratorType;
	
	//! Set up the runner, loading the problem, the configuration, etc.
	Runner(const EngineOptions& options, ProblemGeneratorType generator);
	
	//! Run the search engine
	int run();

protected:
	//! The command-line options for this run
	const EngineOptions _options;
	
	//! The concrete instance generator
	ProblemGeneratorType _generator;
	
	//! The runner starting time
	float _start_time;
	
	//! Print out some information about the characteristics of the problem
	static void report_stats(const Problem& problem, const std::string& out_dir);
};

} } // namespaces
