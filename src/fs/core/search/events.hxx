
#pragma once

#include <iomanip>

#include <fs/core/utils/printers/vector.hxx>
#include <fs/core/utils/config.hxx>
#include <lapkt/tools/events.hxx>
#include <lapkt/tools/logging.hxx>


namespace fs0 { namespace language { namespace fstrips { class Formula; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 {

//! An observer to report and store some stats about the search process
template <typename NodeT, typename StatsT>
class StatsObserver: public lapkt::events::EventHandler {
public:
	using OpenEvent = lapkt::events::NodeOpenEvent<NodeT>;
	using GoalEvent = lapkt::events::GoalFoundEvent<NodeT>;
	using CreationEvent = lapkt::events::NodeCreationEvent<NodeT>;
	using ExpansionEvent = lapkt::events::NodeExpansionEvent<NodeT>;

	StatsObserver(StatsT& stats, bool verbose = true) :
		_stats(stats), _verbose(verbose)
	{
		// Register a call to a member method
		registerEventHandler<OpenEvent>(std::bind(&StatsObserver::open, this, std::placeholders::_1, std::placeholders::_2));
		registerEventHandler<GoalEvent>(std::bind(&StatsObserver::goal, this, std::placeholders::_1, std::placeholders::_2));
		registerEventHandler<CreationEvent>(std::bind(&StatsObserver::creation, this, std::placeholders::_1, std::placeholders::_2));
		registerEventHandler<ExpansionEvent>(std::bind(&StatsObserver::expansion, this, std::placeholders::_1, std::placeholders::_2));
	}

protected:
	void open(lapkt::events::Subject&, const lapkt::events::Event& event) {
		if (_verbose) {
			auto& node = static_cast<const OpenEvent&>(event).node;

            if (!node.has_parent()) {
                LPT_INFO("cout", node);
				_unused(node);

//                LPT_EDEBUG("search", std::setw(7) << "OPEN: " << node);
            }
		}
	}

	void goal(lapkt::events::Subject&, const lapkt::events::Event& event) {
		if (_verbose) {
			LPT_INFO("search", "Goal found");
		}
	}

	void creation(lapkt::events::Subject&, const lapkt::events::Event& event) {
		_stats.generation();
//		if (_verbose) {
//            LPT_EDEBUG("search", std::setw(7) << "GENER.: " << dynamic_cast<const CreationEvent&>(event).node);
//		}

// 		if (_stats.generated() % 10 == 0) {
// 			LPT_INFO("search", "Number of generated nodes: " << _stats.generated());
// 		}

	}

	void expansion(lapkt::events::Subject&, const lapkt::events::Event& event) {
		_stats.expansion();
		if (_verbose) {
//			LPT_EDEBUG("search", std::setw(7) << "EXPAND: " << dynamic_cast<const ExpansionEvent&>(event).node);
		}
// 		if (_stats.expanded() % 10 == 0) {
// 			LPT_INFO("search", "Number of expanded nodes: " << _stats.expanded());
// 		}
	}

	StatsT& _stats;
	bool _verbose;
};

} // namespaces
