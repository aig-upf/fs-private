#pragma once

#include <utils/system.hxx>

namespace fs0 {
	class Config;
	class Problem;
	class SimpleStateModel;
}

namespace fs0 { namespace drivers {

//! A brief interface for any engine creator
class Driver {
public:
	virtual ~Driver() = default;

	//! Perform the search
	virtual ExitCode search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) = 0;
};

class EmbeddedDriver {
public:
	virtual ~EmbeddedDriver() = default;

	//! Perform the search
	virtual ExitCode search(const SimpleStateModel& problem, const Config& config, const std::string& out_dir, float start_time) = 0;

};

}}
