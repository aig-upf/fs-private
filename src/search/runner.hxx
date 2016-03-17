
#pragma once

#include <search/options.hxx>
#include <lib/rapidjson/document.h>

namespace fs0 { class Problem; }

namespace fs0 { namespace engines {
	
//! A basic runner script to bootstrap the problem and run the search engine
class Runner {
public:
	//! The type of the concrete instance generator function
	typedef std::function<void (const rapidjson::Document&, const std::string&)> ProblemGeneratorType;
	
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
};

} } // namespaces
