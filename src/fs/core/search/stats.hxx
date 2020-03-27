
#pragma once
#include <string>
#include <tuple>
#include <vector>

namespace fs0 { 

class SearchStats {
public:
	SearchStats() : _expanded(0), _generated(0), _evaluated(0), _initial_search_time(-1) {}
	
	void expansion() { ++_expanded; }
	void generation(std::size_t distance) {
	    if (distance >= _generated_at_distance.size()) {
            _generated_at_distance.resize(distance+1, 0);
	    }
	    ++_generated_at_distance[distance];
	    ++_generated;
	}
	void evaluation() { ++_evaluated; }

	unsigned long expanded() const { return _expanded; }
	unsigned long generated() const { return _generated; }
	unsigned long evaluated() const { return _evaluated; }

	unsigned long generated_until_last_layer() const {
        auto size = _generated_at_distance.size();
	    if (size <= 1) return 0;
	    assert(_generated_at_distance.back()>0);
	    unsigned long total = 0;
	    for (std::size_t i = 0; i < size-1; ++i) total += _generated_at_distance[i];
	    return total;
	}
	
	using DataPointT = std::tuple<std::string, std::string, std::string>;
	std::vector<DataPointT> dump() const {
		return {
			std::make_tuple("expanded", "Expansions", std::to_string(expanded())),
			std::make_tuple("generated", "Generations", std::to_string(generated())),
            std::make_tuple("generated_until_last_layer", "Generations until last layer", std::to_string(generated_until_last_layer())),
			std::make_tuple("evaluated", "Evaluations", std::to_string(evaluated()))
		};
	}

    void log_start_of_search(double initial_search_time) {
        _initial_search_time = initial_search_time;
    }

	double initial_search_time() const {
	    if (_initial_search_time < 0) {
            throw std::runtime_error("Attempted to access initial_search_time without having invoked"
                                     " method 'log_start_of_search'");
	    }
	    return _initial_search_time;
	}
	
protected:
	unsigned long _expanded;
	unsigned long _generated;
    std::vector<unsigned long> _generated_at_distance;
	unsigned long _evaluated;
	double _initial_search_time;
};

} // namespaces
