#pragma once

#include <models/simple_state_model.hxx>
#include <utils/archive/json.hxx>
#include <utils/system.hxx>
#include <string>

namespace fs0 {
	class Config;
	class Problem;
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

	//! Engine bootstrapping
	virtual void prepare(const SimpleStateModel& problem, const Config& config, const std::string& out_dir) = 0;

	//! Dispose of the current engine
	virtual void dispose() = 0;

	//! Search for a plan (requires driver to have been prepared)
	virtual ExitCode search() = 0;

	//! Perform the search
	virtual ExitCode search(const SimpleStateModel& problem, const Config& config, const std::string& out_dir, float start_time) = 0;

	//! search results
	std::vector<typename SimpleStateModel::ActionType::IdType> plan;
	bool 			solved = false;
	bool 			oom = false;
	float 			search_time = false;
	float 			total_planning_time = 0.0f;
	bool			valid = false;
	float			gen_speed = 0.0;
	float 			eval_speed = 0.0;
	unsigned 		peak_memory = 0;
	ExitCode		result = ExitCode::UNSOLVABLE;

	virtual void archive_results_JSON(std::string filename);
	virtual void archive_scalar_stats( rapidjson::Document& doc );

protected:

	virtual void reset_results();

};

}}
