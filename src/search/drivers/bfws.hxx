
#pragma once

#include <memory>

#include <search/drivers/registry.hxx>
#include <search/nodes/bfws_node.hxx>
#include <search/components/unsat_goals_novelty.hxx>
#include <search/algorithms/aptk/events.hxx>
#include <search/algorithms/aptk/best_first_search.hxx>
#include <search/stats.hxx>
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

std::ostream& operator<<(std::ostream &o, BFWSConfig::Type type);


class BFWSDriver : public Driver {
public:
	//! We use a specific search node
	using NodeT = BFWSNode<fs0::State>;
	
	using NoveltyHeuristic = UnsatGoalsNoveltyComponent<NodeT>;
	
	using Engine = std::unique_ptr<lapkt::StlBestFirstSearch<NodeT, NoveltyHeuristic, GroundStateModel>>;
	
	GroundStateModel setup(Problem& problem) const;
	
	void search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) override;


protected:
	
	std::unique_ptr<NoveltyHeuristic> _heuristic;
	
	std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;
	
	SearchStats _stats;
	
	Engine create_f0(const Config& config, BFWSConfig& bfws_config, const NoveltyFeaturesConfiguration& feature_configuration, const GroundStateModel& model);
};

} } // namespaces
