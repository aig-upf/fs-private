
#pragma once

#include <fs/core/fs_types.hxx>
#include <unordered_map>
#include <utility>

namespace fs0 { class Problem; }

namespace fs0::drivers {

class EngineOptions {
public:
	EngineOptions() = default;
	EngineOptions(int argc, char** argv);

	unsigned getTimeout() const { return _timeout; }

	const std::string& getDataDir() const { return _data_dir; }
	void setDataDir(std::string s ) { _data_dir = std::move(s); }

	const std::string& getOutputDir() const { return _output_dir; }
	void setOutputDir(std::string s) { _output_dir = std::move(s); }

	const std::string& getPlanfile() const { return _planfile; }
	void setPlanfile(std::string s) { _planfile = std::move(s); }

	const std::string& getDefaultConfigurationFilename() const { return _defaults; }
	void setDefaultConfigurationFilename(std::string s ) { _defaults = std::move(s); }

	const std::string& getDriver() const { return _driver; }
	void setDriver(std::string s ) { _driver = std::move(s); }

	const std::unordered_map<std::string, std::string>& getUserOptions() const { return _user_options; }
	std::string getUserOption(const std::string& option) const { return _user_options.at(option); }
	void setUserOption(const std::string& option, std::string value ) { _user_options[option] = std::move(value); }

protected:
	unsigned _timeout{};

	std::string _data_dir;

	std::string _defaults;

	std::string _output_dir;

	std::string _planfile;

	std::string _driver;

	std::unordered_map<std::string, std::string> _user_options;
};

} // namespaces
