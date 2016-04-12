
#pragma once

#include <unordered_map>
#include <fs_types.hxx>
#include <state_model.hxx>

#include <aptk2/search/interfaces/search_algorithm.hxx>

namespace fs0 {
	class Config; 
}

namespace fs0 { namespace drivers {

typedef aptk::SearchAlgorithm<FS0StateModel> FS0SearchAlgorithm;

//! A brief interface for any engine creator
class EngineCreator {
public:
	typedef const EngineCreator* cptr;
	
	virtual ~EngineCreator() {}
	
	//! Create an engine for the given model as specified by the given configuration
	virtual std::unique_ptr<FS0SearchAlgorithm> create(const Config& config, const FS0StateModel& model) const = 0;
};


//! A registry for different types of search drivers
class EngineRegistry {
public:
	~EngineRegistry();
	
	//! The singleton accessor
	static EngineRegistry& instance();
	
	//! Register a new engine creator responsible for creating drivers with the given engine_name
	void add(const std::string& engine_name, EngineCreator::cptr creator);
	
	//! Retrieve the engine creater adequate for the given engine name
	EngineCreator::cptr get(const std::string& engine_name) const;
	
	
protected:
	EngineRegistry();
	
	std::unordered_map<std::string, EngineCreator::cptr> _creators;
};

} } // namespaces
