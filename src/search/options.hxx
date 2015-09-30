
#pragma once

#include <fs0_types.hxx>

namespace fs0 { class Problem; }

namespace fs0 { namespace engines {

class EngineOptions {
public:
	EngineOptions(int argc, char** argv);

	unsigned getTimeout() const { return _timeout; }
	
	const std::string& getDataDir() const { return _data_dir; }
	
	const std::string& getOutputDir() const { return _output_dir; }
	
	const std::string& getConfig() const { return _config; }
	
protected:
	unsigned _timeout;
	
	std::string _data_dir;
	
	std::string _config;
	
	std::string _output_dir;
};

} } // namespaces
