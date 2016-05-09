
#pragma once

#include <fs_types.hxx>
#include <unordered_map>

namespace fs0 { class Problem; }

namespace fs0 { namespace drivers {

class EngineOptions {
public:
	EngineOptions(int argc, char** argv);

	unsigned getTimeout() const { return _timeout; }
	
	const std::string& getDataDir() const { return _data_dir; }
	
	const std::string& getOutputDir() const { return _output_dir; }
	
	const std::string& getDefaultConfigurationFilename() const { return _defaults; }
	
	const std::string& getDriver() const { return _driver; }
	
	const std::unordered_map<std::string, std::string>& getUserOptions() const { return _user_options; }
	
protected:
	unsigned _timeout;
	
	std::string _data_dir;
	
	std::string _defaults;
	
	std::string _output_dir;
	
	std::string _driver;
	
	std::unordered_map<std::string, std::string> _user_options;
};

} } // namespaces
