
#pragma once
#include <string>
#include <tuple>
#include <vector>

namespace fs0 { 

class SearchStats {
public:
	SearchStats() : _expanded(0), _generated(0), _evaluated(0), _initial_search_time(0) {}
	
	void expansion() { ++_expanded; }
	void generation() { ++_generated; }
	void evaluation() { ++_evaluated; }

	unsigned long expanded() const { return _expanded; }
	unsigned long generated() const { return _generated; }
	unsigned long evaluated() const { return _evaluated; }
	
	using DataPointT = std::tuple<std::string, std::string, std::string>;
	std::vector<DataPointT> dump() const {
		return {
			std::make_tuple("expanded", "Expansions", std::to_string(expanded())),
			std::make_tuple("generated", "Generations", std::to_string(generated())),
			std::make_tuple("evaluated", "Evaluations", std::to_string(evaluated()))
		};
	}

    void set_initial_search_time(double t) { _initial_search_time = t; }
	double initial_search_time() const { return _initial_search_time; }
	
protected:
	unsigned long _expanded;
	unsigned long _generated;
	unsigned long _evaluated;
	double _initial_search_time;
};

} // namespaces
