
#pragma once

#include <fs/core/utils/config.hxx>
#include <fs/core/utils/system.hxx>

#include <lapkt/tools/events.hxx>
#include <lapkt/tools/logging.hxx>
#include <lapkt/tools/resources_control.hxx>

#include <iomanip>



namespace fs0 {

//! An observer to report and store some stats about the search process
template <typename NodeT, typename StatsT>
class StatsObserver: public lapkt::events::EventHandler {
public:
	using OpenEvent = lapkt::events::NodeOpenEvent<NodeT>;
	using GoalEvent = lapkt::events::GoalFoundEvent<NodeT>;
	using CreationEvent = lapkt::events::NodeCreationEvent<NodeT>;
	using ExpansionEvent = lapkt::events::NodeExpansionEvent<NodeT>;
    using GenerationEvent = lapkt::events::NodeGenerationEvent<NodeT>;

	explicit StatsObserver(StatsT& stats, bool verbose = false) :
		_stats(stats), _verbose(verbose)
	{
		// Register a call to a member method
		registerEventHandler<OpenEvent>(std::bind(&StatsObserver::open, this, std::placeholders::_1, std::placeholders::_2));
		registerEventHandler<GoalEvent>(std::bind(&StatsObserver::goal, this, std::placeholders::_1, std::placeholders::_2));
        registerEventHandler<GenerationEvent>(std::bind(&StatsObserver::generation, this, std::placeholders::_1, std::placeholders::_2));
		registerEventHandler<CreationEvent>(std::bind(&StatsObserver::creation, this, std::placeholders::_1, std::placeholders::_2));
		registerEventHandler<ExpansionEvent>(std::bind(&StatsObserver::expansion, this, std::placeholders::_1, std::placeholders::_2));
	}

protected:
	void open(lapkt::events::Subject&, const lapkt::events::Event& event) {
		if (_verbose) {
			auto& node = static_cast<const OpenEvent&>(event).node;
            _unused(node);

//            LPT_DEBUG("search", node);
//            LPT_DEBUG("search", "Mem. usage: " << get_current_memory_in_kb() << "kB. (peak: " << get_peak_memory_in_kb() << " kB.)");

            if (!node.has_parent()) {
                LPT_INFO("cout", node);
            }
		}
	}

	void goal(lapkt::events::Subject&, const lapkt::events::Event& event) {
		if (_verbose) {
			LPT_INFO("search", "Goal found");
		}
	}

	void creation(lapkt::events::Subject&, const lapkt::events::Event& event) {

	}

    void generation(lapkt::events::Subject&, const lapkt::events::Event& event) {
        const auto& downcasted = dynamic_cast<const GenerationEvent&>(event);
        _stats.generation(downcasted.node.g);

//        LPT_DEBUG("cout", std::setw(7) << "GENER.: " << downcasted.node);

        if (_verbose) {
            auto generated = _stats.generated();
            if (generated % 50000 == 0) {
                LPT_INFO("cout", "Node generation rate after " << generated / 1000 << "K generations (nodes/sec.): " << node_generation_rate()
                                                               << ". Memory consumption: "<< get_current_memory_in_kb() << "kB. / " << get_peak_memory_in_kb() << " kB.");
            }
        }
    }

    float node_generation_rate() {
	    return _stats.generated() * 1.0 / (aptk::time_used() - _stats.initial_search_time());
	}

	void expansion(lapkt::events::Subject&, const lapkt::events::Event& event) {
		_stats.expansion();
		if (_verbose) {
			LPT_EDEBUG("search", std::setw(7) << "EXPAND: " << dynamic_cast<const ExpansionEvent&>(event).node);
		}
// 		if (_stats.expanded() % 10 == 0) {
// 			LPT_INFO("search", "Number of expanded nodes: " << _stats.expanded());
// 		}
	}

	StatsT& _stats;
	bool _verbose;
};

//! An observer that evaluates the helpfulness of a given node, given the relaxed plan computed
//! in the parent node.
template <typename NodeT>
class HelpfulObserver: public lapkt::events::EventHandler {
public:
	using CreationEvent = lapkt::events::NodeCreationEvent<NodeT>;

	HelpfulObserver() {
		// Register a call to a member method
		registerEventHandler<CreationEvent>(std::bind(&HelpfulObserver::creation, this, std::placeholders::_1, std::placeholders::_2));
	}

protected:
	void creation(lapkt::events::Subject&, const lapkt::events::Event& event) {
		NodeT& node = dynamic_cast<const CreationEvent&>(event).node;
		mark_as_helpful(node);
	}


	//! Returns true iff the given 'state' expanded from 'parent' has been expanded through a helpful action,
	//! i.e. produces at least one of the atoms that support the actions in the first layer of the relaxed plan.
	bool mark_as_helpful(NodeT& node) const {
		const auto& parent = node.parent;
		const auto& state = node.state;

		if (!parent) { // The root node of the search is always helpful!
			node.mark_as_helpful();
			return true;
		}

//		LPT_EDEBUG("heuristic", "Analysing node helpfulness of " << *parent << std::endl << "Relevant atoms:  " << print::container(parent->get_relevant()));
		for (const Atom& atom:parent->get_relevant()) {
			if (state.contains(atom)) {
				node.mark_as_helpful();
				return true;
			}
		}
		return false;
	}
};

//! An observer that decides when to evaluate the heuristic value of a node, when
//! to inherit it from the parent's value, etc.
template <typename NodeT, typename HeuristicT, typename StatsT>
class EvaluationObserver: public lapkt::events::EventHandler {
public:
	using EvaluationT = Config::EvaluationT;
	using ExpansionEvent = lapkt::events::NodeExpansionEvent<NodeT>;
	using CreationEvent  = lapkt::events::NodeCreationEvent<NodeT>;

	EvaluationObserver(HeuristicT& heuristic, EvaluationT evaluation, StatsT& stats) : _heuristic(heuristic), _evaluation(evaluation), _stats(stats) {
		registerEventHandler<ExpansionEvent>(std::bind(&EvaluationObserver::expansion, this, std::placeholders::_1, std::placeholders::_2));
		registerEventHandler<CreationEvent>(std::bind(&EvaluationObserver::creation, this, std::placeholders::_1, std::placeholders::_2));
	}

protected:
	HeuristicT& _heuristic;
	EvaluationT _evaluation;
	StatsT& _stats;

	//! Returns true if the evaluation type is such that the node should be evaluated eagerly, i.e. upon creation
	bool do_early_evaluation(NodeT& node) const {
		if (!node.parent) return true; // Always evaluate eagerly the root node.
		if (_evaluation == EvaluationT::eager) return true;
		if (_evaluation == EvaluationT::delayed_for_unhelpful && node.is_helpful()) return true;
		return false;
	}

	void expansion(lapkt::events::Subject&, const lapkt::events::Event& event) {
		auto& node = dynamic_cast<const ExpansionEvent&>(event).node;
		// If we didn't evaluate the node early, we do it know
		if (!do_early_evaluation(node)) {
			evaluate(node);
		}
	}

	void creation(lapkt::events::Subject&, const lapkt::events::Event& event) {
		auto& node = dynamic_cast<const CreationEvent&>(event).node;
		if (do_early_evaluation(node)) {
			LPT_EDEBUG("heuristic", "Node " << node.hash() << " is evaluated eagerly");
			evaluate(node);
		} else {
			LPT_EDEBUG("heuristic", "Node " << node.hash() << " will be evaluated lazily");
			node.inherit_heuristic_estimate();
		}
	}

	void evaluate(NodeT& node) {
		node.evaluate_with(_heuristic);
		_stats.evaluation();
	}
};

} // namespaces
