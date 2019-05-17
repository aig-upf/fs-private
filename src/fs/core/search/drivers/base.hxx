#pragma once

#include <fs/core/models/simple_state_model.hxx>
#include <fs/core/utils/system.hxx>
#include <string>

namespace fs0 {
	class Config;
	class Problem;
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
