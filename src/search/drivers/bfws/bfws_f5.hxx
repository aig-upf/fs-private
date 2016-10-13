
#pragma once

#include <memory>

#include <search/drivers/bfws/bfws_base.hxx>
#include <search/drivers/registry.hxx>
#include <search/drivers/smart_effect_driver.hxx>
#include <search/nodes/bfws_node.hxx>
#include <search/components/unsat_goals_novelty.hxx>
#include <search/algorithms/aptk/events.hxx>
#include <search/algorithms/aptk/best_first_search.hxx>
#include <search/stats.hxx>
#include <heuristics/relaxed_plan/smart_rpg.hxx>
#include <aptk2/search/components/stl_unsorted_fifo_open_list.hxx>

namespace fs0 { class GroundStateModel; class Config; }

namespace fs0 { namespace drivers {


class BFWSF5Node {
public:
	using ptr_t = std::shared_ptr<BFWSF5Node>;
	
	State state;
	GroundAction::IdType action;
	
	ptr_t parent;

	//! Accummulated cost
	unsigned g;

	//! Novelty of the state
	unsigned novelty;
	
	//! Number of unachieved goals (#g)
	unsigned unachieved;
	
	
	//! The number of atoms in the last relaxed plan computed in the way to the current state that have been
	//! made true along the path (#r)
	enum class REACHED_STATUS : unsigned char {IRRELEVANT_ATOM, UNREACHED, REACHED};
	using Atomset = std::vector<REACHED_STATUS>;
	
	Atomset _reached_idx;
	unsigned _num_relaxed_achieved;
	
	//!
	long hff;
	
public:
	BFWSF5Node() = delete;
	~BFWSF5Node() = default;
	
	BFWSF5Node(const BFWSF5Node& other) = delete;
	BFWSF5Node(BFWSF5Node&& other) = delete;
	BFWSF5Node& operator=(const BFWSF5Node& rhs) = delete;
	BFWSF5Node& operator=(BFWSF5Node&& rhs) = delete;
	
	//! Constructor with full copying of the state (expensive)
	BFWSF5Node(const State& s) : BFWSF5Node(State(s), GroundAction::invalid_action_id, nullptr) {}

	//! Constructor with move of the state (cheaper)
	BFWSF5Node(State&& _state, GroundAction::IdType action_, ptr_t parent_) :
		state(std::move(_state)), action(action_), parent(parent_), g(parent ? parent->g+1 : 0),
		novelty(std::numeric_limits<unsigned>::max()),
		unachieved(std::numeric_limits<unsigned>::max()),
		_num_relaxed_achieved(0),
		hff(std::numeric_limits<long>::max())
	{}

	bool has_parent() const { return parent != nullptr; }
	
	//! Required for the interface of some algorithms that might prioritise helpful actions.
	bool is_helpful() const { return false; }


	bool operator==( const BFWSF5Node& o ) const { return state == o.state; }

	std::size_t hash() const { return state.hash(); }

	//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const BFWSF5Node& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const { 
		std::string hval = hff == std::numeric_limits<long>::max() ? "?" : std::to_string(hff);
		return os << "{@ = " << this << ", s = " << state << ", g = " << g << ", w = " << novelty << ", #g=" << unachieved << ", #r=" << _num_relaxed_achieved << ", h = " << hval << ", parent = " << parent << "}";
	}
	
	void update_reached_counters(const State& state) {
		const ProblemInfo& info = ProblemInfo::getInstance();
		const TupleIndex& tuple_idx = Problem::getInstance().get_tuple_index();
		
		// We copy the map of reached values from the parent node and will update it below
		_reached_idx = parent->_reached_idx;
		_num_relaxed_achieved = parent->_num_relaxed_achieved;
		
		for (VariableIdx var = 0; var < state.numAtoms(); ++var) {
			
			// We perform the check only for functional variables or non-false predicative variables,
			// as 0-valued predicative variables won't be part of the set of atoms anyway.
			// TODO This should be revised if we change the definition of the set of relevant atoms
			// TODO and it includes possible 0-valued predicative atoms
			if (!info.isPredicativeVariable(var) || state.getValue(var) != 0) {
				TupleIdx index = tuple_idx.to_index(Atom(var, state.getValue(var)));
				auto& element = _reached_idx[index];
				if (element == REACHED_STATUS::UNREACHED) {
					element = REACHED_STATUS::REACHED;
					++_num_relaxed_achieved;
				}
			}
		}
	}
	
	template <typename Heuristic>
	void evaluate_with( Heuristic& ensemble ) {
		unachieved = ensemble.get_unachieved(this->state);
		
		if (!has_parent() || unachieved < parent->unachieved) {
			// TODO Is the initialization of _num_relaxed_achieved correct?
			hff = ensemble.compute_heuristic(state, _reached_idx, _num_relaxed_achieved);
		} else {
			update_reached_counters(this->state);
		}
		
		novelty = ensemble.novelty(state, unachieved, _num_relaxed_achieved);
		if (novelty > ensemble.max_novelty()) {
			novelty = std::numeric_limits<unsigned>::max();
		}
	}
	
	void inherit_heuristic_estimate() {
		if (parent) {
			novelty = parent->novelty;
			hff = parent->hff;
			unachieved = parent->unachieved;
			_reached_idx = parent->_reached_idx;
			_num_relaxed_achieved = parent->_num_relaxed_achieved;
		}
	}
	
	//! What to do when an 'other' node is found during the search while 'this' node is already in
	//! the open list
	void update_in_open_list(ptr_t other) {
		if (other->g < this->g) {
			this->g = other->g;
			this->action = other->action;
			this->parent = other->parent;
			this->novelty = other->novelty;
			this->hff = other->hff;
			unachieved = other->unachieved;
			_num_relaxed_achieved = other->_num_relaxed_achieved;
			_reached_idx = other->_reached_idx;
		}
	}

	bool dead_end() const { return hff == -1; }
};



struct F5NodeComparer {
	using NodePtrT = std::shared_ptr<BFWSF5Node>;
	bool operator()(const NodePtrT& n1, const NodePtrT& n2) const {
		if (n1->novelty > n2->novelty ) return true;
		if (n1->novelty < n2->novelty ) return false;
		if (n1->unachieved > n2->unachieved) return true;
		if (n1->unachieved < n2->unachieved) return false;
		return n1->g > n2->g;
	}
};



template <typename StateModelT, typename BaseHeuristicT>
class BFWSF5HeuristicEnsemble {
public:
	BFWSF5HeuristicEnsemble(const StateModelT& model, unsigned max_novelty, const NoveltyFeaturesConfiguration& feature_configuration, std::unique_ptr<BaseHeuristicT>&& heuristic) :
		_problem(model.getTask()),
		_feature_configuration(feature_configuration),
		_max_novelty(max_novelty), 
		_novelty_evaluators(),
		_base_heuristic(std::move(heuristic)),
		_unsat_goal_atoms_heuristic(model)
	{}
	
	~BFWSF5HeuristicEnsemble() = default;
	
	inline unsigned max_novelty() { return _max_novelty; }

	long compute_heuristic(const State& state, BFWSF5Node::Atomset& relevant, unsigned& num_relevant) { 
		std::vector<Atom> relevant_atoms;
		long h = _base_heuristic->evaluate(state, relevant_atoms);
		num_relevant = relevant_atoms.size();
		relevant = build_atomset(relevant_atoms);
		LPT_DEBUG("heuristic" , std::endl << "Computed heuristic value of " << h <<  " for state: " << std::endl << state << std::endl << "****************************************");
		return h;
	}
	
	//! Builds an atomset from a set of relevant atoms - all possible atoms set to irrelevant except those considered as relevant,
	//! which are set to unreached
	BFWSF5Node::Atomset build_atomset(const std::vector<Atom>& relevant) {
		const TupleIndex& tuple_idx = Problem::getInstance().get_tuple_index();
		BFWSF5Node::Atomset atomset(tuple_idx.size(), BFWSF5Node::REACHED_STATUS::IRRELEVANT_ATOM);
		for (const Atom& atom:relevant) {
			atomset[tuple_idx.to_index(atom)] = BFWSF5Node::REACHED_STATUS::UNREACHED;
		}
		return atomset;
	}
	
	//! Compute the novelty of the state wrt all the states with the same heuristic value.
	unsigned novelty(const State& state, unsigned unachieved, unsigned relaxed_achieved) {
// 		auto index = std::make_pair(unachieved, relaxed_achieved);
		auto ind = index(unachieved, relaxed_achieved);
		auto it = _novelty_evaluators.find(ind);
		if (it == _novelty_evaluators.end()) {
			auto inserted = _novelty_evaluators.insert(std::make_pair(ind, GenericNoveltyEvaluator(_problem, _max_novelty, _feature_configuration)));
			it = inserted.first;
		}
		return it->second.evaluate(state);
	}
	
	unsigned get_unachieved(const State& state) {
		return _unsat_goal_atoms_heuristic.evaluate(state);
	}
	
	static long index(unsigned unachieved, unsigned relaxed_achieved) {
		return (unsigned short)(relaxed_achieved<<16) | (unsigned short)(unachieved);
	}
	
protected:
	const Problem& _problem;
	
	const NoveltyFeaturesConfiguration& _feature_configuration;
	
	unsigned _max_novelty;

	//! We have one different novelty evaluators for each actual heuristic value that a node might have.
	std::unordered_map<long, GenericNoveltyEvaluator> _novelty_evaluators;
	
	std::unique_ptr<BaseHeuristicT> _base_heuristic;
	
	//! An UnsatisfiedGoalAtomsHeuristic to count the number of unsatisfied goals
	UnsatisfiedGoalAtomsHeuristic<StateModelT> _unsat_goal_atoms_heuristic;
};

using BFWS_F5_BASE_HEURISTIC = unsigned;
using BFWS_F5_ENSEMBLE = unsigned;


template <typename BaseHeuristicT>
class BFWSF5Subdriver {
public:
	using NodeT = BFWSF5Node;
	using NodeCompareT = F5NodeComparer;
	using HeuristicEnsembleT = BFWSF5HeuristicEnsemble<GroundStateModel, BaseHeuristicT>;
	using RawEngineT = lapkt::StlBestFirstSearch<NodeT, HeuristicEnsembleT, GroundStateModel, std::shared_ptr<NodeT>, NodeCompareT>;
	using EngineT = std::unique_ptr<RawEngineT>;

	//!
	EngineT create(const Config& config, BFWSConfig& bfws_config, const NoveltyFeaturesConfiguration& feature_configuration, const GroundStateModel& model) {
	
		using EvaluatorT = EvaluationObserver<NodeT, HeuristicEnsembleT>;
		using StatsT = StatsObserver<NodeT>;

		auto base_heuristic = std::unique_ptr<BaseHeuristicT>(SmartEffectDriver::configure_heuristic(model.getTask(), config));
		_heuristic = std::unique_ptr<HeuristicEnsembleT>(new HeuristicEnsembleT(model, bfws_config._max_width, feature_configuration, std::move(base_heuristic)));

		_handlers.push_back(std::unique_ptr<StatsT>(new StatsT(_stats)));
		_handlers.push_back(std::unique_ptr<EvaluatorT>(new EvaluatorT(*_heuristic, config.getNodeEvaluationType())));
		
		auto engine = EngineT(new RawEngineT(model, *_heuristic));
		lapkt::events::subscribe(*engine, _handlers);
		return engine;
	}
	
	const SearchStats& getStats() const { return _stats; }

protected:
	//!
	std::unique_ptr<HeuristicEnsembleT> _heuristic;
	
	//!
	std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;
	
	//!
	SearchStats _stats;
};


using BFWS_F5 = BFWSF5Subdriver<gecode::SmartRPG>;


} } // namespaces
