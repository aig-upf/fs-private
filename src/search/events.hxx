
#pragma once

#include <search/algorithms/ehc_gbfs.hxx>
#include <search/nodes/heuristic_search_node.hxx>
#include <utils/config.hxx>

#include <search/algorithms/ehc.hxx>
#include <search/algorithms/aptk/events.hxx>
#include <heuristics/relaxed_plan/smart_rpg.hxx>
#include <search/stats.hxx>

namespace fs0 { namespace language { namespace fstrips { class Formula; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 {

//! An observer to report and store some stats about the search process
template <typename NodeT>
class StatsObserver: public lapkt::events::EventHandler {
public:
	using OpenEvent = lapkt::events::NodeOpenEvent<NodeT>;
	using GoalEvent = lapkt::events::GoalFoundEvent<NodeT>;
	using CreationEvent = lapkt::events::NodeCreationEvent<NodeT>;
	using ExpansionEvent = lapkt::events::NodeExpansionEvent<NodeT>;

	StatsObserver(SearchStats& stats) :
		_stats(stats)
	{
		// Register a call to a member method
		registerEventHandler<OpenEvent>(std::bind(&StatsObserver::open, this, std::placeholders::_1, std::placeholders::_2));
		registerEventHandler<GoalEvent>(std::bind(&StatsObserver::goal, this, std::placeholders::_1, std::placeholders::_2));
		registerEventHandler<CreationEvent>(std::bind(&StatsObserver::creation, this, std::placeholders::_1, std::placeholders::_2));
		registerEventHandler<ExpansionEvent>(std::bind(&StatsObserver::expansion, this, std::placeholders::_1, std::placeholders::_2));
	}

protected:
	void open(lapkt::events::Subject&, const lapkt::events::Event& event) {
		auto& node = dynamic_cast<const OpenEvent&>(event).node;
		LPT_DEBUG("cout", node);
		_unused(node);
	}
	
	void goal(lapkt::events::Subject&, const lapkt::events::Event& event) {
		LPT_INFO("cout", "Goal found");
	}
	
	void creation(lapkt::events::Subject&, const lapkt::events::Event& event) {
		_stats.generation();
	}
	
	void expansion(lapkt::events::Subject&, const lapkt::events::Event& event) {
		_stats.expansion();
	}
	
	SearchStats& _stats;
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
		
		LPT_EDEBUG("heuristic", "Analysing node helpfulness of " << *parent << std::endl << "Relevant atoms:  " << print::container(parent->get_relevant()));
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
template <typename NodeT, typename HeuristicT>
class EvaluationObserver: public lapkt::events::EventHandler {
public:
	using EvaluationT = Config::EvaluationT;
	using ExpansionEvent = lapkt::events::NodeExpansionEvent<NodeT>;
	using CreationEvent  = lapkt::events::NodeCreationEvent<NodeT>;
	
	EvaluationObserver(HeuristicT& heuristic, EvaluationT evaluation) : _heuristic(heuristic), _evaluation(evaluation) {
		registerEventHandler<ExpansionEvent>(std::bind(&EvaluationObserver::expansion, this, std::placeholders::_1, std::placeholders::_2));
		registerEventHandler<CreationEvent>(std::bind(&EvaluationObserver::creation, this, std::placeholders::_1, std::placeholders::_2));
	}
	
protected:
	HeuristicT& _heuristic;
	EvaluationT _evaluation;
	
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
		if (!do_early_evaluation(node)) node.evaluate_with(_heuristic);
	}
	
	void creation(lapkt::events::Subject&, const lapkt::events::Event& event) {
		auto& node = dynamic_cast<const CreationEvent&>(event).node;
		if (do_early_evaluation(node)) {
			LPT_EDEBUG("heuristic", "Node " << node.hash() << " is evaluated eagerly");
			node.evaluate_with(_heuristic);
		} else {
			LPT_EDEBUG("heuristic", "Node " << node.hash() << " will be evaluated lazily");
			node.inherit_heuristic_estimate();
		}
	}
};

} // namespaces
