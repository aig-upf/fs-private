
#pragma once

#include <memory>

#include <search/drivers/registry.hxx>
#include <search/nodes/bfws_node.hxx>
#include <search/components/unsat_goals_novelty.hxx>
#include <search/algorithms/aptk/events.hxx>
#include <search/algorithms/aptk/best_first_search.hxx>
#include <search/stats.hxx>
#include <heuristics/relaxed_plan/smart_rpg.hxx>
#include <aptk2/search/components/stl_unsorted_fifo_open_list.hxx>

namespace fs0 { class GroundStateModel; class Config; }

namespace fs0 { namespace drivers {

class BFWSConfig {
public:
	enum class Type {F0, F1, F2, F5};
	
	BFWSConfig(const Config& config);
	BFWSConfig(const BFWSConfig&) = default;
	BFWSConfig& operator=(const BFWSConfig&) = default;
	BFWSConfig(BFWSConfig&&) = default;
	BFWSConfig& operator=(BFWSConfig&&) = default;

	//!
	const Type _type;
	
	//!
	const unsigned _max_width;
	
protected:
	static Type parse_type(const std::string& type);
};

//! Print a BFWSConfig type
std::ostream& operator<<(std::ostream &o, BFWSConfig::Type type);


template <typename StateModelT>
class BFWSDriver : public Driver {
public:
	void search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) override;
};


} } // namespaces
