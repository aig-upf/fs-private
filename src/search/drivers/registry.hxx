
#pragma once

#include <unordered_map>
#include <fs_types.hxx>
#include <ground_state_model.hxx>
#include <lifted_state_model.hxx>

#include <aptk2/search/interfaces/search_algorithm.hxx>

namespace fs0 {
	class Config; 
}

namespace fs0 { namespace drivers {

typedef aptk::SearchAlgorithm<GroundStateModel> FSGroundSearchAlgorithm;
typedef aptk::SearchAlgorithm<LiftedStateModel> FSLiftedSearchAlgorithm;

//! A brief interface for any engine creator
class Driver {
public:
	virtual ~Driver() = default;
	
	//! Perform the search
	virtual void search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) = 0;
};


//! A registry for different types of search drivers
class EngineRegistry {
public:
	~EngineRegistry();
	
	//! The singleton accessor
	static EngineRegistry& instance();
	
	//! Register a new engine creator responsible for creating drivers with the given engine_name
	void add(const std::string& engine_name, Driver* creator);
	
	//! Retrieve the engine creater adequate for the given engine name
	Driver* get(const std::string& engine_name);
	
	
protected:
	EngineRegistry();
	
	std::unordered_map<std::string, Driver*> _creators;
};

} } // namespaces
