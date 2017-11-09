
#pragma once

#include <unordered_map>
#include <fs/core/search/drivers/base.hxx>

namespace fs0 {
	class Config;
	class Problem;
}

namespace fs0 { namespace drivers {

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
