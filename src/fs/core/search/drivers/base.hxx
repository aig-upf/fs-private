#pragma once

namespace fs0 {
	class Config;
	class Problem;
    enum class ExitCode : int;
}

namespace fs0::drivers {

class EngineOptions;

//! A brief interface for any engine creator
class Driver {
public:
	virtual ~Driver() = default;

	//! Perform the search
	virtual ExitCode search(Problem& problem, const Config& config, const EngineOptions& options, float start_time) = 0;
};

}
