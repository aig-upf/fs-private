
#pragma once

#include <fs/core/fs_types.hxx>
#include <unordered_map>

namespace fs0 { class Problem; }

namespace fs0 { namespace drivers {

class EngineOptions {
public:
	EngineOptions() = default;
	EngineOptions(int argc, char** argv);

	unsigned getTimeout() const { return _timeout; }

	const std::string& getDataDir() const { return _data_dir; }
	void setDataDir( std::string s ) { _data_dir = s; }

	const std::string& getOutputDir() const { return _output_dir; }
	void setOutputDir( std::string s) { _output_dir = s; }

	const std::string& getDefaultConfigurationFilename() const { return _defaults; }
	void setDefaultConfigurationFilename( std::string s ) { _defaults = s; }

	const std::string& getDriver() const { return _driver; }
	void setDriver( std::string s ) { _driver = s; }

	const std::unordered_map<std::string, std::string>& getUserOptions() const { return _user_options; }
	std::string getUserOption(std::string option) const { return _user_options.at(option); }
	void setUserOption( std::string option, std::string value ) { _user_options[option] = value; }

protected:
	unsigned _timeout;

	std::string _data_dir;

	std::string _defaults;

	std::string _output_dir;

	std::string _driver;

	std::unordered_map<std::string, std::string> _user_options;
};

} } // namespaces
