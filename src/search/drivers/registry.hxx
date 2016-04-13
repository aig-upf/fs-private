
#pragma once

#include <unordered_map>
#include <fs_types.hxx>
#include <ground_state_model.hxx>

#include <aptk2/search/interfaces/search_algorithm.hxx>

namespace fs0 {
	class Config; 
}

namespace fs0 { namespace drivers {

typedef aptk::SearchAlgorithm<GroundStateModel> FS0SearchAlgorithm;

//! A brief interface for any engine creator
class Driver {
public:
	typedef const Driver* cptr;
	
	virtual ~Driver() {}
	
	//! Create an engine for the given model as specified by the given configuration
	virtual std::unique_ptr<FS0SearchAlgorithm> create(const Config& config, const GroundStateModel& model) const = 0;
	
	virtual GroundStateModel setup(const Config& config, Problem& problem) const;
};


//! A registry for different types of search drivers
class EngineRegistry {
public:
	~EngineRegistry();
	
	//! The singleton accessor
	static EngineRegistry& instance();
	
	//! Register a new engine creator responsible for creating drivers with the given engine_name
	void add(const std::string& engine_name, Driver::cptr creator);
	
	//! Retrieve the engine creater adequate for the given engine name
	Driver::cptr get(const std::string& engine_name) const;
	
	
protected:
	EngineRegistry();
	
	std::unordered_map<std::string, Driver::cptr> _creators;
};

} } // namespaces
