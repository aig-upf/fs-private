
#pragma once

#include <stdio.h>
#include <unordered_set>



#include <problem.hxx>
#include "base.hxx"
#include "stats.hxx"
#include <search/drivers/sbfws/relevant_atomset.hxx>
#include <utils/printers/vector.hxx>
#include <utils/printers/actions.hxx>
#include <lapkt/search/components/open_lists.hxx>
#include <utils/config.hxx>
#include <lapkt/novelty/tuples.hxx>
#include <lapkt/novelty/features.hxx>
#include <lapkt/tools/resources_control.hxx>
#include <lapkt/tools/logging.hxx>
#include <heuristics/novelty/features.hxx>
#include <heuristics/novelty/goal_ball_filter.hxx>

// For writing the R sets
#include <utils/archive/json.hxx>

namespace fs0 { namespace bfws {

using FSFeatureValueT = lapkt::novelty::FeatureValueT;
typedef lapkt::novelty::Width1Tuple<FSFeatureValueT> Width1Tuple;
typedef lapkt::novelty::Width1TupleHasher<FSFeatureValueT> Width1TupleHasher;
typedef lapkt::novelty::Width2Tuple<FSFeatureValueT> Width2Tuple;
typedef lapkt::novelty::Width2TupleHasher<FSFeatureValueT> Width2TupleHasher;

template <typename StateT, typename ActionType>
class MultiValuedIWRunNode {
public:
	using ActionT = ActionType;
	using PT = std::shared_ptr<MultiValuedIWRunNode<StateT, ActionT>>;

	//! The state in this node
	StateT state;

	//! The action that led to this node
	typename ActionT::IdType action;

	//! The parent node
	PT parent;

	//! Accummulated cost
	unsigned g;

	//! The novelty  of the state
	unsigned char _w;

	std::vector<std::pair<Width1Tuple, Width1Tuple>> _nov2_pairs;

	//! The generation order, uniquely identifies the node
	//! NOTE We're assuming we won't generate more than 2^32 ~ 4.2 billion nodes.
	uint32_t _gen_order;


	MultiValuedIWRunNode() = default;
	~MultiValuedIWRunNode() = default;
	MultiValuedIWRunNode(const MultiValuedIWRunNode&) = default;
	MultiValuedIWRunNode(MultiValuedIWRunNode&&) = delete;
	MultiValuedIWRunNode& operator=(const MultiValuedIWRunNode&) = delete;
	MultiValuedIWRunNode& operator=(MultiValuedIWRunNode&&) = delete;

	//! Constructor with full copying of the state (expensive)
	MultiValuedIWRunNode(const StateT& s, unsigned long gen_order) : MultiValuedIWRunNode(StateT(s), ActionT::invalid_action_id, nullptr, gen_order) {}

	//! Constructor with move of the state (cheaper)
	MultiValuedIWRunNode(StateT&& _state, typename ActionT::IdType _action, PT _parent, uint32_t gen_order) :
		state(std::move(_state)),
		action(_action),
		parent(_parent),
		g(parent ? parent->g+1 : 0),
		_w(std::numeric_limits<unsigned char>::max()),
		_gen_order(gen_order)
	{
		assert(_gen_order > 0); // Very silly way to detect overflow, in case we ever generate > 4 billion nodes :-)
	}


	bool has_parent() const { return parent != nullptr; }

	//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const MultiValuedIWRunNode<StateT, ActionT>& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const {
		os << "{@ = " << this;
		os << ", #=" << _gen_order ;
		os << ", s = " << state ;
		os << ", g=" << g ;
		os << ", w=" << (_w == std::numeric_limits<unsigned char>::max() ? "INF" : std::to_string(_w));

		os << ", act=" << action ;
		os << ", parent = " << (parent ? "#" + std::to_string(parent->_gen_order) : "None");
		return os;
	}

	bool operator==( const MultiValuedIWRunNode<StateT, ActionT>& o ) const { return state == o.state; }

	std::size_t hash() const { return state.hash(); }
};


template <typename NodeT, typename FeatureSetT, typename NoveltyEvaluatorT>
class MultiValuedSimulationEvaluator {
protected:
	//! The set of features used to compute the novelty
	const FeatureSetT& _features;

	//! A single novelty evaluator will be in charge of evaluating all nodes
	std::unique_ptr<NoveltyEvaluatorT> _evaluator;

public:

    typedef typename NoveltyEvaluatorT::ValuationT ValuationT;

	MultiValuedSimulationEvaluator(const FeatureSetT& features, NoveltyEvaluatorT* evaluator) :
		_features(features),
		_evaluator(evaluator)
	{}

	~MultiValuedSimulationEvaluator() = default;

	//! Returns false iff we want to prune this node during the search
	unsigned evaluate(NodeT& node) {
		if (node.parent) {
			// Important: the novel-based computation works only when the parent has the same novelty type and thus goes against the same novelty tables!!!
			node._w = _evaluator->evaluate(_features.evaluate(node.state), _features.evaluate(node.parent->state));
		} else {
			node._w = _evaluator->evaluate(_features.evaluate(node.state));
		}

		return node._w;
	}

	std::vector<Width1Tuple> reached_tuples() const {
		std::vector<Width1Tuple> tuples;
		_evaluator->mark_tuples_in_novelty1_table(tuples);
		return tuples;
	}

	void reset() {
		_evaluator->reset();
	}

    const FeatureSetT& feature_set() const { return _features; }

};


//! A single IW run (with parametrized max. width) that runs until (independent)
//! satisfaction of each of the provided goal atoms, and computes the set
//! of atoms R that is relevant for the achievement of at least one atom.
//! R is computed treating the actions as a black-box. For this, an atom is considered
//! relevant for a certain goal atom if that atom is true in at least one of the states
//! that lies on the path between the seed node and the first node where that goal atom is
//! satisfied.
template <typename NodeT,
          typename StateModel,
          typename NoveltyEvaluatorT,
		  typename FeatureSetT
>
class MultiValuedIWRun
{
public:
	using ActionT = typename StateModel::ActionType;
	using StateT = typename StateModel::StateT;

	using ActionIdT = typename StateModel::ActionType::IdType;
	using NodePT = std::shared_ptr<NodeT>;

	using SimEvaluatorT = MultiValuedSimulationEvaluator<NodeT, FeatureSetT, NoveltyEvaluatorT>;

	using FeatureValueT = typename NoveltyEvaluatorT::FeatureValueT;

	using OpenListT = lapkt::SimpleQueue<NodeT>;

	struct Config {
		//! Whether to perform a complete run or a partial one, i.e. up until (independent) satisfaction of all goal atoms.
		bool _complete;

		//! The maximum levels of novelty to be considered
		unsigned _max_width;

		//!
		const fs0::Config& _global_config;

		//! Whether to extract goal-informed relevant sets R
		bool _goal_directed;

		//!
		bool _force_adaptive_run;

		//!
		bool _force_R_all;

		//!
		bool _r_g_prime;

		//!
		unsigned _gr_actions_cutoff;

		//! Enforce state constraints
		bool _enforce_state_constraints;

		//! Load R set from file
		std::string _R_file;

		//! Goal Ball filtering
		bool _filter_R_set;

		//! Log search
		bool _log_search;

		Config(bool complete, bool mark_negative, unsigned max_width, const fs0::Config& global_config) :
			_complete(complete),
			_max_width(max_width),
			_global_config(global_config),
			_goal_directed(global_config.getOption<bool>("sim.goal_directed", false)),
			_force_adaptive_run(global_config.getOption<bool>("sim.hybrid", false)),
			_force_R_all(global_config.getOption<bool>("sim.r_all", false)),
			_r_g_prime(global_config.getOption<bool>("sim.r_g_prime", false)),
			_gr_actions_cutoff(global_config.getOption<unsigned>("sim.act_cutoff", std::numeric_limits<unsigned>::max())),
			_enforce_state_constraints(global_config.getOption<bool>("sim.enforce_state_constraints", false)),
			_R_file(global_config.getOption<std::string>("sim.from_file", "")),
			_filter_R_set(global_config.getOption<bool>("sim.filter", false)),
			_log_search(global_config.getOption<bool>("sim.log", false))
		{}
	};

protected:
	//! The search model
	const StateModel& _model;

	//! The simulation configuration
	Config _config;

	//!
	std::vector<NodePT> _optimal_paths;

	//! '_unreached' contains the indexes of all those goal atoms that have yet not been reached.
	std::unordered_set<unsigned> _unreached;

	//! Contains the indexes of all those goal atoms that were already reached in the seed state
	std::vector<bool> _in_seed;

	//! A single novelty evaluator will be in charge of evaluating all nodes
	SimEvaluatorT _evaluator;

	//! Some node counts
	uint32_t _generated;
	uint32_t _w1_nodes_expanded;
	uint32_t _w2_nodes_expanded;
	uint32_t _w1_nodes_generated;
	uint32_t _w2_nodes_generated;
	uint32_t _w_gt2_nodes_generated;

	//! The general statistics of the search
	BFWSStats& _stats;

	//! Whether to print some useful extra information or not
	bool _verbose;

	// MRJ: IW(1) debugging
	std::vector<NodePT>	_visited;

public:

	//! Constructor
	MultiValuedIWRun(const StateModel& model, const FeatureSetT& featureset, NoveltyEvaluatorT* evaluator, const MultiValuedIWRun::Config& config, BFWSStats& stats, bool verbose) :
		_model(model),
		_config(config),
		_optimal_paths(model.num_subgoals()),
		_unreached(),
		_in_seed(),
		_evaluator(featureset, evaluator),
		_generated(1),
		_w1_nodes_expanded(0),
		_w2_nodes_expanded(0),
		_w1_nodes_generated(0),
		_w2_nodes_generated(0),
		_w_gt2_nodes_generated(0),
		_stats(stats),
		_verbose(verbose)
	{
	}

	void reset() {
		std::vector<NodePT> _(_optimal_paths.size(), nullptr);
		_optimal_paths.swap(_);
		_generated = 1;
		_w1_nodes_expanded = 0;
		_w2_nodes_expanded = 0;
		_w1_nodes_generated = 0;
		_w2_nodes_generated = 0;
		_w_gt2_nodes_generated = 0;
		_evaluator.reset();
	}

	~MultiValuedIWRun() = default;

	// Disallow copy, but allow move
	MultiValuedIWRun(const MultiValuedIWRun&) = delete;
	MultiValuedIWRun(MultiValuedIWRun&&) = default;
	MultiValuedIWRun& operator=(const MultiValuedIWRun&) = delete;
	MultiValuedIWRun& operator=(MultiValuedIWRun&&) = default;


	//! Mark all tuples in the path to some goal. 'seed_nodes' contains all nodes satisfying some subgoal.
	void mark_tuples_in_path_to_subgoal(const std::vector<NodePT>& seed_nodes, std::unordered_set<Width1Tuple,Width1TupleHasher>& tuples) const {
		std::unordered_set<NodePT> all_visited;

		for (NodePT node:seed_nodes) {

			NodePT root = node;
			// We ignore s0
			while (node->has_parent()) {
				// If the node has already been processed, no need to do it again, nor to process the parents,
				// which will necessarily also have been processed.
				auto res = all_visited.insert(node);
				if (!res.second) break;

				const StateT& state = node->state;
				const StateT& parent_state = node->parent->state;
				_unused(state);
                const FeatureSetT& phi = _evaluator.feature_set();
                typename SimEvaluatorT::ValuationT parent_phi_S = phi.evaluate(parent_state);
                typename SimEvaluatorT::ValuationT phi_S = phi.evaluate(state);

				for (const auto& p_q : node->_nov2_pairs) {
                    Width1Tuple p, q;
                    std::tie(p,q) = p_q;

                    assert( phi_S[p.first] == p.second );
                    assert( phi_S[q.first] == q.second );
                    assert( parent_phi_S[p.first] == p.second);
                    assert( parent_phi_S[q.first] == p.second);  // Otherwise the tuple couldn't be new

                    if ( parent_phi_S[p.first] == p.second ) {
                        tuples.insert(p);
                    } else if ( parent_phi_S[q.first] == q.second ) {
                        tuples.insert(q);
                    } else {
                        tuples.insert(p);
                        tuples.insert(q);
                    }
				}
				node = node->parent;
			}
		}
	}

	std::unordered_set<Width1Tuple,Width1TupleHasher> mark_all_tuples_in_path_to_subgoal(const std::vector<NodePT>& seed_nodes) const {
		std::unordered_set<Width1Tuple,Width1TupleHasher> tuples;
		std::unordered_set<NodePT> all_visited;

		for (NodePT node:seed_nodes) {
			// We ignore s0
			while (node->has_parent()) {
				// If the node has already been processed, no need to do it again, nor to process the parents,
				// which will necessarily also have been processed.
				auto res = all_visited.insert(node);
				if (!res.second) break;

				const StateT& state = node->state;
                const FeatureSetT& phi = _evaluator.feature_set();
                typename SimEvaluatorT::ValuationT phi_S = phi.evaluate(state);
                for ( unsigned k = 0; k < phi_S.size(); k++ )
                    tuples.insert(Width1Tuple(k,phi_S[k]));

				node = node->parent;
			}
		}
		return tuples;
	}

	void report_simulation_stats(float simt0) {
		_stats.simulation();
		_stats.sim_add_time(aptk::time_used() - simt0);
		_stats.sim_add_expanded_nodes(_w1_nodes_expanded+_w2_nodes_expanded);
		_stats.sim_add_generated_nodes(_w1_nodes_generated+_w2_nodes_generated+_w_gt2_nodes_generated);
		_stats.reachable_subgoals( _model.num_subgoals() - _unreached.size());
	}

	std::vector<Width1Tuple> compute_R_all() {
        std::vector<Width1Tuple> dummy;
        throw std::runtime_error("Multi Valued IW Simulation: R_All is not well defined");
        return dummy;
	}

	std::vector<Width1Tuple> compute_R(const StateT& seed) {

		if ( !_config._R_file.empty() ) {
			std::vector<Width1Tuple> R;
			load_R_set( _config._R_file, R);
			LPT_INFO("search", "Simulation - R set has been loaded from a given file: " << _config._R_file );
			LPT_INFO("search", "\t Loaded " << R.size() << " entries from file...");
			return R;
		}

		if (_config._force_R_all) {
			if (_verbose) LPT_INFO("search", "Simulation - R=R[All] is the user-preferred option");
			return compute_R_all();
		}

		if (_config._r_g_prime) {
			if (_verbose) LPT_INFO("search", "Simulation - R=R'_G is the user-preferred option");
			return compute_R_g_prime(seed);
		}

		if (_config._force_adaptive_run) {
			return compute_adaptive_R(seed);
		} else if (_config._max_width == 1){
			return compute_plain_R1(seed);
		} else if (_config._max_width == 2){
			return compute_plain_RG2(seed);
		} else {
			throw std::runtime_error("Simulation max_width too high");
		}
	}

	std::vector<Width1Tuple> compute_plain_RG2(const StateT& seed) {
		assert(_config._max_width == 2);

		_config._complete = false;
		float simt0 = aptk::time_used();
  		run(seed, _config._max_width);
		report_simulation_stats(simt0);
		if (_config._goal_directed && _unreached.size() == 0) {
			LPT_INFO("search", "Simulation - IW(" << _config._max_width << ") run reached " << _model.num_subgoals() - _unreached.size() << " goals");
			return extract_R_G(false);
		}
		// Else, compute the goal-unaware version of R containing all atoms seen during the IW run
		return extract_R_1();

	}


	std::vector<Width1Tuple> compute_plain_R1(const StateT& seed) {
		assert(_config._max_width == 1);
		_config._complete = false;
		float simt0 = aptk::time_used();

  		run(seed, _config._max_width);
		report_simulation_stats(simt0);

		if (_config._goal_directed && _unreached.size() == 0) {
			LPT_INFO("search", "Simulation - IW(" << _config._max_width << ") reached all subgoals, computing R_G[" << _config._max_width << "]");
			return extract_R_G_1();
		}

		// Else, compute the goal-unaware version of R containing all atoms seen during the IW run
		return extract_R_1();

	}

	// MRJ: This is not very useful at the moment, since we do really need to map the
	// floating point numbers exactly back from the file... which
	// means than otherwise than the number 0.0 (or numbers very close to
	// it) we can't find a match on loaded R set.
	template <typename Container>
	void load_R_set( std::string filename, Container& R ) {
		using namespace rapidjson;
		//const ProblemInfo& info = ProblemInfo::getInstance();
		FILE* fp = fopen(filename.c_str(), "rb"); // non-Windows use "r"
		char readBuffer[65536];
		FileReadStream is(fp, readBuffer, sizeof(readBuffer));
		if (fp == nullptr ) {
			throw std::runtime_error("MultiValuedIWRun::load_R_set: Failed to open file '" + filename + "'");
		}
        Document R_set;
		R_set.ParseStream(is);
		fclose(fp);

		const Value& tuples = R_set["elements"];
		assert(tuples.IsArray());
		for (SizeType i = 0; i < tuples.Size(); i++) {// Uses SizeType instead of size_t
			// MRJ: input file format consists of feature index and feature (raw) value
			unsigned feature_index = tuples[i][0].GetInt();
			FSFeatureValueT feature_value = tuples[i][1].GetInt();
			Width1Tuple t_i( feature_index, feature_value );
			R.push_back(t_i);
		}
	}

	void filter_R_set( const std::vector<Width1Tuple>& R, std::vector<Width1Tuple>& filtered_R ) {
		GoalBallFilter filter;

		for ( Width1Tuple f : R) {
			unsigned j;
			int v;
			std::tie(j,v) = f;

			auto phi = dynamic_cast<const StateVariableFeature*>(_evaluator.feature_set().at(j));
			if ( phi == nullptr ) {// not a state variable feature
				filtered_R.push_back(f);
				continue;
			}
			object_id o = make_object(phi->codomain(), v);
			if ( o_type(o) != type_id::int_t && o_type(o) != type_id::float_t ) { // not a feature we know how to filter
				filtered_R.push_back(f);
				continue;
			}
			auto scope = phi->scope();

			filter.add_sample( j, scope[0], o );
		}
		filter.filter_samples();
		filter.get_filtered_set<std::vector<Width1Tuple>, FSFeatureValueT >(filtered_R);
	}

	std::vector<Width2Tuple>
	compute_coupled_features(const std::vector<Width1Tuple>& R) {
		GoalBallFilter filter;

		for ( Width1Tuple f : R) {
			unsigned j;
			int v;
			std::tie(j,v) = f;

			auto phi = dynamic_cast<const StateVariableFeature*>(_evaluator.feature_set().at(j));
			if ( phi == nullptr ) {// not a state variable feature
				continue;
			}
			object_id o = make_object(phi->codomain(), v);
			if ( o_type(o) != type_id::int_t && o_type(o) != type_id::float_t ) { // not a feature we know how to filter
				continue;
			}
			auto scope = phi->scope();

			filter.add_sample( j, scope[0], o );
		}
		filter.filter_samples();
		std::vector<Width2Tuple> coupled;
		filter.get_output<std::vector<Width2Tuple>, FSFeatureValueT >(coupled);
		return coupled;
	}

	template <typename Container>
	void dump_R_set( const Container& R, std::string filename ) {
		using namespace rapidjson;

		const ProblemInfo& info = ProblemInfo::getInstance();
        Document R_set;
        Document::AllocatorType& allocator = R_set.GetAllocator();
        R_set.SetObject();
        Value domainName;
        domainName.SetString(StringRef(info.getDomainName().c_str()));
        R_set.AddMember("domain", domainName.Move(), allocator );
        Value instanceName;
        instanceName.SetString(StringRef(info.getInstanceName().c_str()));
        R_set.AddMember("instance", instanceName.Move(), allocator );

		Value elements(kArrayType);
        {
			for ( Width1Tuple f : R) {
				unsigned j;
				int v;
				std::tie(j,v) = f;

				auto phi = dynamic_cast<const Feature*>(_evaluator.feature_set().at(j));

				// MRJ: State variable features always come first

				auto scope = phi->scope();
				if (scope.empty()) continue; // Scope not available for this feature

				Value entry(kArrayType); {

					if ( scope.size() == 1 ) {
						Value name;
                        name.SetString( StringRef( info.getVariableName(scope[0]).c_str() ));
						entry.PushBack(name.Move(),allocator);
					} else {
						Value tuple(kArrayType);{
							for ( unsigned l = 0; l < scope.size(); l++ ) {
								Value name;
		                        name.SetString( StringRef( info.getVariableName(scope[l]).c_str() ));
								tuple.PushBack(name.Move(),allocator);
							}
						}
						entry.PushBack(tuple.Move(),allocator);
					}
					Value feature_id(j);
					entry.PushBack(feature_id.Move(),allocator);
					Value value;
					object_id o = make_object(phi->codomain(), v);
					if ( o_type(o) == type_id::bool_t )
						value = Value( fs0::value<bool>(o) );
					else if ( o_type(o) == type_id::int_t )
						value = Value( fs0::value<int>(o) );
					else if ( o_type(o) == type_id::float_t )
						value = Value( fs0::value<float>(o) );
					else {
						std::string _literal = info.object_name(o);
						value.SetString( _literal.c_str(), _literal.size(), allocator ) ;
					}
					entry.PushBack(value.Move(),allocator);
					Value raw_value(v);
					entry.PushBack(raw_value.Move(),allocator);

				}
				elements.PushBack(entry.Move(), allocator);
			}
        }
        R_set.AddMember("elements", elements, allocator);

		FILE* fp = fopen(filename.c_str(), "wb"); // non-Windows use "w"
        char writeBuffer[65536];
        FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
        Writer<FileWriteStream> writer(os);
        R_set.Accept(writer);
        fclose(fp);
	}

	std::vector<Width1Tuple> extract_R_1() {
		std::vector<Width1Tuple> R = _evaluator.reached_tuples();
		LPT_INFO("search", "Simulation - IW(" << _config._max_width << ") run reached " << _model.num_subgoals() - _unreached.size() << " goals");
		if (_verbose) {
			unsigned c = R.size();
			LPT_INFO("search", "Simulation - |R[1]| = " << c);
			_stats.relevant_atoms(c);
			dump_R_set( R, "R1.set.json");

		}
		return R;
	}

	std::vector<Width1Tuple> compute_R_g_prime(const StateT& seed) {
		_config._complete = false;


		float simt0 = aptk::time_used();
  		run(seed, 1);
		report_simulation_stats(simt0);

		if (_unreached.size() == 0) {
			std::vector<NodePT> seed_nodes = extract_seed_nodes();
			std::unordered_set<Width1Tuple,Width1TupleHasher> R_G = mark_all_tuples_in_path_to_subgoal(seed_nodes);
			unsigned R_G_size = R_G.size();
			LPT_INFO("search", "Simulation - IW(1) run reached all goals");
			LPT_INFO("search", "Simulation - |R_G'[1]| = " << R_G_size << " (computed from " << seed_nodes.size() << " subgoal-reaching nodes)");
			_stats.relevant_atoms(R_G_size);
			dump_R_set( R_G, "RGprime1.set.json");
			return std::vector<Width1Tuple>(R_G.begin(),R_G.end());

		}

		LPT_INFO("search", "Simulation - IW(1) run did not reach all goals, throwing IW(2) simulation");


		if (_config._gr_actions_cutoff < std::numeric_limits<unsigned>::max()) {
			unsigned num_actions = Problem::getInstance().getGroundActions().size();
			if (num_actions > _config._gr_actions_cutoff) { // Too many actions to compute IW(º2)
				LPT_INFO("search", "Simulation - Number of actions (" << num_actions << " > " << _config._gr_actions_cutoff << ") considered too high to run IW(2).");
				return compute_R_all();
			} else {
				LPT_INFO("search", "Simulation - Number of actions (" << num_actions << " <= " << _config._gr_actions_cutoff << ") considered low enough to run IW(2).");
			}
		}

		reset();
		run(seed, 2);
		report_simulation_stats(simt0);
		_stats.reachable_subgoals( _model.num_subgoals() - _unreached.size());

		if (_unreached.size() != 0 )
			LPT_INFO("search", "Simulation - IW(2) run did not reach all goals");
		if (_unreached.empty())
			LPT_INFO("search", "Simulation - IW(2) run reached all goals");

		std::vector<NodePT> seed_nodes = extract_seed_nodes();
		std::unordered_set<Width1Tuple,Width1TupleHasher> R_G = mark_all_tuples_in_path_to_subgoal(seed_nodes);
		unsigned R_G_size = R_G.size();

		LPT_INFO("search", "Simulation - |R_G'[2]| = " << R_G_size << " (computed from " << seed_nodes.size() << " subgoal-reaching nodes)");
		_stats.relevant_atoms(R_G_size);
		dump_R_set( R_G, "RGprime2.set.json");
		return std::vector<Width1Tuple>(R_G.begin(),R_G.end());
	}



	std::vector<Width1Tuple> compute_adaptive_R(const StateT& seed) {
		_config._complete = false;


		float simt0 = aptk::time_used();
  		run(seed, 1);
		report_simulation_stats(simt0);

		if (_unreached.size() == 0) {
			// If a single IW[1] run reaches all subgoals, we return R=emptyset
			LPT_INFO("search", "Simulation - IW(1) run reached all goals, thus R={}");
			_stats.r_type(0);
			return std::vector<Width1Tuple>();
		} else {
			LPT_INFO("search", "Simulation - IW(1) run did not reach all goals, throwing IW(2) simulation");
		}

		// Otherwise, run IW(2)
		reset();
		run(seed, 2);
		report_simulation_stats(simt0);
		_stats.reachable_subgoals( _model.num_subgoals() - _unreached.size());

		return extract_R_G(false);
	}

	//! Extracts the goal-oriented set of relevant atoms after a simulation run
	std::vector<Width1Tuple> extract_R_G(bool r_all_fallback) {

		if (r_all_fallback) {
			unsigned num_subgoals = _model.num_subgoals();
			unsigned initially_reached = std::count(_in_seed.begin(), _in_seed.end(), true);
			unsigned reached_by_simulation = num_subgoals - _unreached.size() - initially_reached;
			if (_verbose) LPT_INFO("search", "Simulation - " << reached_by_simulation << " subgoals were newly reached by the simulation.");
			bool decide_r_all = (reached_by_simulation < (0.5*num_subgoals));
			decide_r_all = _unreached.size() != 0; // XXX Use R_All is any non-reached
			if (decide_r_all) {
				if (_verbose) LPT_INFO("search", "Simulation - Falling back to R=R[All]");
				_stats.r_type(1);
				return compute_R_all();
			} else {
				if (_verbose) LPT_INFO("search", "Simulation - Computing R_G");
			}
		}


		std::vector<NodePT> seed_nodes = extract_seed_nodes();
		std::unordered_set<Width1Tuple,Width1TupleHasher> R_G;
		mark_tuples_in_path_to_subgoal(seed_nodes, R_G);

		unsigned R_G_size = R_G.size();
		if (_verbose) {
			LPT_INFO("search", "Simulation - |R_G[" << _config._max_width << "]| = " << R_G_size << " (computed from " << seed_nodes.size() << " subgoal-reaching nodes)");

		}
		_stats.relevant_atoms(R_G_size);
		_stats.r_type(2);
		std::stringstream ss;
		ss << "RG" << _config._max_width << ".set.json";
		dump_R_set( R_G, ss.str());
		return std::vector<Width1Tuple>(R_G.begin(),R_G.end());
	}

	std::vector<NodePT> extract_seed_nodes() {
		std::vector<NodePT> seed_nodes;
		for (unsigned subgoal_idx = 0; subgoal_idx < _optimal_paths.size(); ++subgoal_idx) {
			if (!_in_seed[subgoal_idx] && _optimal_paths[subgoal_idx] != nullptr) {
				seed_nodes.push_back(_optimal_paths[subgoal_idx]);
			}
		}
		return seed_nodes;
	}

	std::vector<Width1Tuple> extract_R_G_1() {
		std::vector<NodePT> seed_nodes = extract_seed_nodes();
		std::unordered_set<Width1Tuple,Width1TupleHasher> R_G = mark_all_tuples_in_path_to_subgoal(seed_nodes);

		unsigned R_G_size = R_G.size();
		if (_verbose) {
			LPT_INFO("search", "Simulation - |R_G[" << _config._max_width << "]| = " << R_G_size << " (computed from " << seed_nodes.size() << " subgoal-reaching nodes)");
		}
		_stats.relevant_atoms(R_G_size);
		_stats.r_type(2);
		dump_R_set( R_G, "RG1.set.json");
		return std::vector<Width1Tuple>(R_G.begin(), R_G.end());
	}



// 	std::vector<AtomIdx> _compute_R(const StateT& seed) {

// 		_config._complete = false;
//
// 		float simt0 = aptk::time_used();
//  		run(seed);
// 		_stats.simulation();
// 		_stats.sim_add_time(aptk::time_used() - simt0);
// 		_stats.sim_add_expanded_nodes(_w1_nodes_expanded+_w2_nodes_expanded);
// 		_stats.sim_add_generated_nodes(_w1_nodes_generated+_w2_nodes_generated+_w_gt2_nodes_generated);
// 		_stats.reachable_subgoals(_model.num_subgoals() - _unreached.size());

// 		std::vector<NodePT> w1_goal_reaching_nodes;
// 		std::vector<NodePT> w2_goal_reaching_nodes;
// 		std::vector<NodePT> wgt2_goal_reaching_nodes;


		/*
		LPT_INFO("search", "Simulation - Number of novelty-1 nodes: " << _w1_nodes.size());
		LPT_INFO("search", "Simulation - Number of novelty=1 nodes expanded in the simulation: " << _w1_nodes_expanded);
		LPT_INFO("search", "Simulation - Number of novelty=2 nodes expanded in the simulation: " << _w2_nodes_expanded);
		LPT_INFO("search", "Simulation - Number of novelty=1 nodes generated in the simulation: " << _w1_nodes_generated);
		LPT_INFO("search", "Simulation - Number of novelty=2 nodes generated in the simulation: " << _w2_nodes_generated);
		LPT_INFO("search", "Simulation - Number of novelty>2 nodes generated in the simulation: " << _w_gt2_nodes_generated);
		LPT_INFO("search", "Simulation - Total number of generated nodes (incl. pruned): " << _generated);
		LPT_INFO("search", "Simulation - Number of seed novelty-1 nodes: " << w1_seed_nodes.size());
		*/

// 		auto relevant_w2_nodes = compute_relevant_w2_nodes();
// 		LPT_INFO("search", "Simulation - Number of relevant novelty-2 nodes: " << relevant_w2_nodes.size());

// 		auto su = compute_union(relevant_w2_nodes); // Order matters!
// 		auto hs = compute_hitting_set(relevant_w2_nodes);

// 		LPT_INFO("search", "Simulation - union-based R (|R|=" << su.size() << ")");
// 		_print_atomset(su);

// 		LPT_INFO("search", "Simulation - hitting-set-based-based R (|R|=" << hs.size() << ")");
// 		_print_atomset(hs);

		//std::vector<AtomIdx> relevant(hs.begin(), hs.end());
// 		std::vector<AtomIdx> relevant(su.begin(), su.end());
// 		std::sort(relevant.begin(), relevant.end());
// 		return relevant;
// 		return {};
// 	}

	class DeactivateZCC {
		bool _current_setting;
	public:
		DeactivateZCC() {
			_current_setting = fs0::Config::instance().getZeroCrossingControl();
			fs0::Config::instance().setZeroCrossingControl(false);
		}

		~DeactivateZCC() {
			fs0::Config::instance().setZeroCrossingControl(_current_setting);
		}
	};

	bool run(const StateT& seed, unsigned max_width) {
		if (_verbose) LPT_INFO("search", "Simulation - Starting IW Simulation");

		std::shared_ptr<DeactivateZCC> zcc_setting = nullptr;
		if (!_config._enforce_state_constraints ) {
			LPT_INFO("search", ":Simulation - Deactivating zero crossing control");
			zcc_setting = std::make_shared<DeactivateZCC>();
		}

		NodePT root = std::make_shared<NodeT>(seed, _generated++);
		mark_seed_subgoals(root);

		auto nov =_evaluator.evaluate(*root);
		assert(nov==1);
		update_novelty_counters_on_generation(nov);

// 		LPT_DEBUG("cout", "Simulation - Seed node: " << *root);

		assert(max_width <= 2); // The current swapping-queues method works only for up to width 2, but is trivial to generalize if necessary

		OpenListT open_w1, open_w2;
		OpenListT open_w1_next, open_w2_next; // The queues for the next depth level.

		open_w1.insert(root);

		while (true) {
			while (!open_w1.empty() || !open_w2.empty()) {
				NodePT current = open_w1.empty() ? open_w2.next() : open_w1.next();

				// Expand the node
				update_novelty_counters_on_expansion(current->_w);

				for (const auto& a : _model.applicable_actions(current->state, _config._enforce_state_constraints)) {
					StateT s_a = _model.next( current->state, a );
					NodePT successor = std::make_shared<NodeT>(std::move(s_a), a, current, _generated++);

					unsigned char novelty = _evaluator.evaluate(*successor);
					update_novelty_counters_on_generation(novelty);

					// LPT_INFO("search", "Simulation - Node generated: " << *successor);
					if (_config._log_search )
						_visited.push_back(successor);


					if (process_node(successor)) {  // i.e. all subgoals have been reached before reaching the bound
						report("All subgoals reached");
						return true;
					}

					if (novelty <= max_width && novelty == 1) open_w1_next.insert(successor);
					else if (novelty <= max_width && novelty == 2) open_w2_next.insert(successor);
				}

			}
			// We've processed all nodes in the current depth level.
			open_w1.swap(open_w1_next);
			open_w2.swap(open_w2_next);

			if (open_w1.empty() && open_w2.empty()) break;
		}

		report("State space exhausted");
		return false;
	}

	void update_novelty_counters_on_expansion(unsigned char novelty) {
		if (novelty == 1) ++_w1_nodes_expanded;
		else if (novelty== 2) ++_w2_nodes_expanded;
	}

	void update_novelty_counters_on_generation(unsigned char novelty) {
		if (novelty==1) ++_w1_nodes_generated;
		else if (novelty==2)  ++_w2_nodes_generated;
		else ++_w_gt2_nodes_generated;
	}

	void report(const std::string& result) const {
		if (!_verbose) return;
		LPT_INFO("search", "Simulation - Result: " << result);
		LPT_INFO("search", "Simulation - Num reached subgoals: " << (_model.num_subgoals() - _unreached.size()) << " / " << _model.num_subgoals());
		LPT_INFO("search", "Simulation - Expanded nodes with w=1 " << _w1_nodes_expanded);
		LPT_INFO("search", "Simulation - Expanded nodes with w=2 " << _w2_nodes_expanded);
		LPT_INFO("search", "Simulation - Generated nodes with w=1 " << _w1_nodes_generated);
		LPT_INFO("search", "Simulation - Generated nodes with w=2 " << _w2_nodes_generated);
		LPT_INFO("search", "Simulation - Generated nodes with w>2 " << _w_gt2_nodes_generated);
		if (! _config._log_search ) return;

		using namespace rapidjson;

		// Dump optimal_paths and visited into JSON document
		const ProblemInfo& info = ProblemInfo::getInstance();
		Document trace;
		Document::AllocatorType& allocator = trace.GetAllocator();
		trace.SetObject();
		Value domainName;
		domainName.SetString(StringRef(info.getDomainName().c_str()));
		trace.AddMember("domain", domainName.Move(), allocator );
		Value instanceName;
		instanceName.SetString(StringRef(info.getInstanceName().c_str()));
		trace.AddMember("instance", instanceName.Move(), allocator );
		Value visits(kArrayType);
        {
            for ( auto n : _visited ) {
				auto s = n->state;
                Value state(kObjectType);
				JSONArchive::store(state, allocator, s);
                {
					Value v(n->_gen_order);
					state.AddMember( "gen_order", v, allocator);
                }
                visits.PushBack(state.Move(), allocator);
            }
        }
        trace.AddMember("visited", visits, allocator);
		Value opt_paths(kArrayType);
		{
			for ( auto path_to_sub_goal : _optimal_paths ) {
				if ( path_to_sub_goal == nullptr ) continue;
				Value path(kArrayType);
				{
					NodePT node = path_to_sub_goal;

					while (node->has_parent()) {
						Value state(kObjectType);
						JSONArchive::store(state, allocator, node->state);
						path.PushBack( state.Move(),allocator);
						node = node->parent;
					}

					Value s0(kObjectType);
					JSONArchive::store( s0, allocator, node->state );
					path.PushBack( s0.Move(), allocator );
				}
				opt_paths.PushBack(path.Move(),allocator);
			}

		}
		trace.AddMember("optimal_paths", opt_paths, allocator );

		FILE* fp = fopen( "mv_iw_run.json", "wb"); // non-Windows use "w"
		char writeBuffer[65536];
		FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
		Writer<FileWriteStream> writer(os);
		trace.Accept(writer);
		fclose(fp);
	}

protected:

	//! Returns true iff all goal atoms have been reached in the IW search
	bool process_node(NodePT& node) {
		if (_config._complete) return process_node_complete(node);

		const StateT& state = node->state;

		// We iterate through the indexes of all those goal atoms that have not yet been reached in the IW search
		// to check if the current node satisfies any of them - and if it does, we mark it appropriately.
		for (auto it = _unreached.begin(); it != _unreached.end(); ) {
			unsigned subgoal_idx = *it;

			if (_model.goal(state, subgoal_idx)) {
// 				node->satisfies_subgoal = true;
// 				_all_paths[subgoal_idx].push_back(node);
				if (!_optimal_paths[subgoal_idx]) _optimal_paths[subgoal_idx] = node;
				it = _unreached.erase(it);
			} else {
				++it;
			}
		}
		// As soon as all nodes have been processed, we return true so that we can stop the search
		return _unreached.empty();
	}

	//! Returns true iff all goal atoms have been reached in the IW search
	bool process_node_complete(NodePT& node) {
		const StateT& state = node->state;

		for (unsigned i = 0; i < _model.num_subgoals(); ++i) {
			if (!_in_seed[i] && _model.goal(state, i)) {
// 				node->satisfies_subgoal = true;
				if (!_optimal_paths[i]) _optimal_paths[i] = node;
				_unreached.erase(i);
			}
		}
 		return _unreached.empty();
		//return false; // return false so we don't interrupt the processing
	}

	void mark_seed_subgoals(const NodePT& node) {
		std::vector<bool> _(_model.num_subgoals(), false);
		_in_seed.swap(_);
		_unreached.clear();
		for (unsigned i = 0; i < _model.num_subgoals(); ++i) {
			if (_model.goal(node->state, i)) {
				_in_seed[i] = true;
			} else {
				_unreached.insert(i);
			}
		}
	}

// public:
// 	const std::unordered_set<NodePT>& get_relevant_nodes() const { return _visited; }
};

} } // namespaces
