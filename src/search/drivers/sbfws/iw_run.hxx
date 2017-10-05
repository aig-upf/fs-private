
#pragma once

#include <stdio.h>
#include <unordered_set>
#include <boost/graph/graph_concepts.hpp>

#include <lapkt/tools/resources_control.hxx>
#include <lapkt/tools/logging.hxx>

#include <problem.hxx>
#include "base.hxx"
#include "stats.hxx"
#include <search/drivers/sbfws/relevant_atomset.hxx>
#include <utils/printers/vector.hxx>
#include <utils/printers/actions.hxx>
#include <lapkt/search/components/open_lists.hxx>
#include <utils/config.hxx>
#include <languages/fstrips/formulae.hxx>

#include <search/utils.hxx>


using NoGoodAtomsSet = std::unordered_set<fs0::Atom>;
using RelevantFeatureSet = std::unordered_set<std::pair<unsigned, int>,  boost::hash<std::pair<unsigned, int>> >;




namespace fs0 { namespace bfws {
	
template <typename StateT, typename ActionType>
class IWRunNode {
public:
	using ActionT = ActionType;
	using PT = std::shared_ptr<IWRunNode<StateT, ActionT>>;

	//! The state in this node
	StateT state;

	//! The action that led to this node
	typename ActionT::IdType action;

	//! The parent node
	PT parent;
	
	//! Accummulated cost
	unsigned g;
	
// 	bool satisfies_subgoal; // Whether the node satisfies some subgoal
	
	//! The novelty  of the state
	unsigned char _w;
	
	//! The indexes of the variables whose atoms form the set 1(s), which contains all atoms in 1(parent(s)) not deleted by the action that led to s, plus those 
	//! atoms in s with novelty 1.
// 	std::vector<unsigned> _nov1atom_idxs;
	
	//! Implicit encoding of the atoms that contribute novelty 1 to the state
// 	boost::dynamic_bitset<> _B_of_s;
	
	//! Whether the path-novely of the node is one
// 	bool _path_novelty_is_1;
	
	std::vector<std::pair<AtomIdx, AtomIdx>> _nov2_pairs;
	
	//! The generation order, uniquely identifies the node
	//! NOTE We're assuming we won't generate more than 2^32 ~ 4.2 billion nodes.
	uint32_t _gen_order;


	IWRunNode() = default;
	~IWRunNode() = default;
	IWRunNode(const IWRunNode&) = default;
	IWRunNode(IWRunNode&&) = delete;
	IWRunNode& operator=(const IWRunNode&) = delete;
	IWRunNode& operator=(IWRunNode&&) = delete;

	//! Constructor with full copying of the state (expensive)
	IWRunNode(const StateT& s, unsigned long gen_order) : IWRunNode(StateT(s), ActionT::invalid_action_id, nullptr, gen_order) {}

	//! Constructor with move of the state (cheaper)
	IWRunNode(StateT&& _state, typename ActionT::IdType _action, PT _parent, uint32_t gen_order) :
		state(std::move(_state)),
//		feature_valuation(0),
		action(_action),
		parent(_parent),
		g(parent ? parent->g+1 : 0),
		_w(std::numeric_limits<unsigned char>::max()),
// 		_path_novelty_is_1(false),
// 		_B_of_s(state.numAtoms()),
		_gen_order(gen_order)
	{
		assert(_gen_order > 0); // Very silly way to detect overflow, in case we ever generate > 4 billion nodes :-)
	}


	bool has_parent() const { return parent != nullptr; }

	//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const IWRunNode<StateT, ActionT>& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const {
// 		const Problem& problem = Problem::getInstance();
		os << "{@ = " << this;
		os << ", #=" << _gen_order ;
		os << ", s = " << state ;
		os << ", g=" << g ;
		//os << ", w=" << (_evaluated ? (_w == std::numeric_limits<unsigned char>::max() ? "INF" : std::to_string(_w)) : "?") ;
		os << ", w=" << (_w == std::numeric_limits<unsigned char>::max() ? "INF" : std::to_string(_w));
		
		os << ", act=" << action ;
// 		if (action < std::numeric_limits<unsigned>::max()) {
// 			os << ", act=" << *(problem.getGroundActions()[action]) ;
// 			os << fs0::print::full_action(*(problem.getGroundActions()[action]));
// 		} else {
// 			os << ", act=" << "NONE" ;
// 		}
		
		os << ", parent = " << (parent ? "#" + std::to_string(parent->_gen_order) : "None");
		return os;
	}

	bool operator==( const IWRunNode<StateT, ActionT>& o ) const { return state == o.state; }

	std::size_t hash() const { return state.hash(); }
};


template <typename NodeT, typename FeatureSetT, typename NoveltyEvaluatorT>
class SimulationEvaluator {
protected:
	//! The set of features used to compute the novelty
	const FeatureSetT& _features;

	//! A single novelty evaluator will be in charge of evaluating all nodes
	std::unique_ptr<NoveltyEvaluatorT> _evaluator;

public:
	SimulationEvaluator(const FeatureSetT& features, NoveltyEvaluatorT* evaluator) :
		_features(features),
		_evaluator(evaluator)
	{}

	~SimulationEvaluator() = default;
	
	//! Returns false iff we want to prune this node during the search
	unsigned evaluate(NodeT& node) {
		if (node.parent) {
			// Important: the novel-based computation works only when the parent has the same novelty type and thus goes against the same novelty tables!!!
			node._w = _evaluator->evaluate(_features.evaluate(node.state), _features.evaluate(node.parent->state));
		} else {
			node._w = _evaluator->evaluate(_features.evaluate(node.state));
		}
		
// 		if (node._w == 2) {
// 			_evaluator->mark_nov2atoms_from_last_state(node._nov2_pairs);
// 		}
		
		return node._w;
	}
	
	std::vector<bool> reached_atoms() const {
		std::vector<bool> atoms;
		_evaluator->mark_atoms_in_novelty1_table(atoms);
		return atoms;
	}
	
	void reset() {
		_evaluator->reset();
	}		
	
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
class IWRun
{
public:
	using ActionT = typename StateModel::ActionType;
	using StateT = typename StateModel::StateT;
	
	using ActionIdT = typename StateModel::ActionType::IdType;
	using NodePT = std::shared_ptr<NodeT>;
	
	using SimEvaluatorT = SimulationEvaluator<NodeT, FeatureSetT, NoveltyEvaluatorT>;
	
	using FeatureValueT = typename NoveltyEvaluatorT::FeatureValueT;
	
	using OpenListT = lapkt::SimpleQueue<NodeT>;
	
	struct Config {
		//! Whether to perform a complete run or a partial one, i.e. up until (independent) satisfaction of all goal atoms.
		bool _complete;
		
		//! Whether to take into account negative propositions or not
		bool _mark_negative;
		
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
		
		Config(bool complete, bool mark_negative, unsigned max_width, const fs0::Config& global_config) :
			_complete(complete),
			_mark_negative(mark_negative),
			_max_width(max_width),
			_global_config(global_config),
			_goal_directed(global_config.getOption<bool>("goal_directed", false)), 
			_force_adaptive_run(global_config.getOption<bool>("sim.hybrid", false)),
			_force_R_all(global_config.getOption<bool>("sim.r_all", false)),
			_r_g_prime(global_config.getOption<bool>("sim.r_g_prime", false)),
			_gr_actions_cutoff(global_config.getOption<unsigned>("sim.act_cutoff", std::numeric_limits<unsigned>::max()))
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
	
	std::vector<bool> _seen_w1_atoms;
	
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
	
	std::unordered_map<unsigned, NodePT> _plans;
	
	NoGoodAtomsSet _relevant_no_good_atoms;
	
	//! The node first achieving each problem tuple
	std::vector<NodePT> _tuple_to_node;
	
	//! Object_id o to the index of the tuple "holding()=o"
	std::vector<AtomIdx> _obj_to_holding_tuple_idx;
	
	const StateT _init;
	
	//Domain dependent
	std::vector<VariableIdx> _all_objects_gtype;
	std::vector<VariableIdx> _all_objects_conf;
	
	//Contains the first node that includes a goal of the form not p, where p is a relevant no good atom.
	std::vector<NodePT> _subgoals_path;

	const FeatureSetT& _featureset;
	

public:

	//! Constructor
	IWRun(const StateModel& model, const FeatureSetT& featureset, NoveltyEvaluatorT* evaluator, const IWRun::Config& config, BFWSStats& stats, bool verbose) :
		_model(model),
		_config(config),
		_optimal_paths(model.num_subgoals()),
		_unreached(),
		_seen_w1_atoms(),
		_in_seed(),
		_evaluator(featureset, evaluator),
		_generated(1),
		_w1_nodes_expanded(0),
		_w2_nodes_expanded(0),
		_w1_nodes_generated(0),
		_w2_nodes_generated(0),		
		_w_gt2_nodes_generated(0),
		_stats(stats),
		_verbose(verbose),
		_tuple_to_node(_model.getTask().get_tuple_index().size(), nullptr),
		_obj_to_holding_tuple_idx(ProblemInfo::getInstance().getNumObjects(), -1),
		_init(model.init()),
		_all_objects_gtype(ProblemInfo::getInstance().getNumObjects(), -1),
		_all_objects_conf(ProblemInfo::getInstance().getNumObjects(), -1),
		_featureset(featureset)

		
	{
	  Problem p(model.getTask());
	  std::cout << "Is tautology: " << p.getStateConstraints()->is_tautology() << std::endl;
	  const AtomIndex& index = Problem::getInstance().get_tuple_index();
	  const ProblemInfo& info = ProblemInfo::getInstance();
	  VariableIdx holding_v = info.getVariableId("holding()");
	  
	  for (ObjectIdx obj:info.getTypeObjects("object_id")) {
	    _all_objects_conf[obj] = derive_config_variable(info, obj);
	    _all_objects_gtype[obj] = derive_gtype_variable(info, obj);
	    AtomIdx t = index.to_index(holding_v, obj); // i.e. the tuple index of the atom holding()=o
	    _obj_to_holding_tuple_idx.at(obj) = t;
	  }
	  	  
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

	~IWRun() = default;
	
	
	NoGoodAtomsSet& get_relevant_no_good_atoms() {return _relevant_no_good_atoms;}

	// Disallow copy, but allow move
	IWRun(const IWRun&) = delete;
	IWRun(IWRun&&) = default;
	IWRun& operator=(const IWRun&) = delete;
	IWRun& operator=(IWRun&&) = default;
	
	
	VariableIdx derive_config_variable(const ProblemInfo& info, ObjectIdx object_id) {
	  std::string obj_name = info.deduceObjectName(object_id, info.getTypeId("object_id"));
	  return info.getVariableId("confo(" + obj_name  +  ")");
	  
	}

	VariableIdx derive_gtype_variable(const ProblemInfo& info, ObjectIdx object_id) {
	  std::string obj_name = info.deduceObjectName(object_id, info.getTypeId("object_id"));
	  return info.getVariableId("gtype(" + obj_name  +  ")");
	  
	}

	
	//! Mark all atoms in the path to some goal. 'seed_nodes' contains all nodes satisfying some subgoal.
	void mark_atoms_in_path_to_subgoal(const std::vector<NodePT>& seed_nodes, std::vector<bool>& atoms) const {
		const AtomIndex& index = Problem::getInstance().get_tuple_index();
		std::unordered_set<NodePT> all_visited;
		assert(atoms.size() == index.size());
		

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
				
				
				for (const auto& p_q:node->_nov2_pairs) {
					Atom p = index.to_atom(p_q.first);
					Atom q = index.to_atom(p_q.second);
					
					assert(state.contains(p));
					assert(state.contains(q));
					assert(!(parent_state.contains(p) && parent_state.contains(q))); // Otherwise the tuple couldn't be new
					
					
					std::vector<AtomIdx> to_mark;
					if (parent_state.contains(p)) {
						if (p.getValue() != 0) {
							atoms[p_q.first] = true; // TODO THIS WON'T GENERALIZE WELL TO FSTRIPS DOMAINS
// 							std::cout << "ATTENTION : Marking atom " << p << std::endl;
						}
					}
					else if (parent_state.contains(q)) {
						if (q.getValue() != 0) {
							atoms[p_q.second] = true;
// 							std::cout << "ATTENTION : Marking atom " << q << std::endl;
						}
					}
					else { // The parent state contains none
// 						std::cout << "ATTENTION : Would Mark pair " << p << ", " << q << std::endl;
						if (p.getValue() != 0) atoms[p_q.first] = true;
						if (q.getValue() != 0) atoms[p_q.second] = true;
					}
					
				}
				
				/*
				const StateT& state = node->state;
				for (unsigned var = 0; var < state.numAtoms(); ++var) {
					if (state.getValue(var) == 0) continue; // TODO THIS WON'T GENERALIZE WELL TO FSTRIPS DOMAINS
					AtomIdx atom = index.to_index(var, state.getValue(var));
					atoms[atom] = true;
				}
				*/
				
				node = node->parent;
			}			
		}
	}
	
	std::vector<bool> mark_all_atoms_in_path_to_subgoal(const std::vector<NodePT>& seed_nodes)  {
		const AtomIndex& index = Problem::getInstance().get_tuple_index();
		std::vector<bool> atoms(index.size(), false);
		std::unordered_set<NodePT> all_visited;
		assert(atoms.size() == index.size());
				
		for (NodePT node:seed_nodes) {
			// We ignore s0
			while (node->has_parent()) {
			  
			 // std::cout << node->state << std::endl;
				
				// If the node has already been processed, no need to do it again, nor to process the parents,
				// which will necessarily also have been processed.
				auto res = all_visited.insert(node);
				if (!res.second) break;
				
				const StateT& state = node->state;
				for (unsigned var = 0; var < state.numAtoms(); ++var) {
					ObjectIdx val = state.getValue(var);
					if (val != 0) {
						atoms[index.to_index(var, val)] = true;
					}
				}
				
				node = node->parent;
			}
		}
		return atoms;
	}
	
	RelevantFeatureSet mark_all_features_in_path_to_subgoal(const std::vector<NodePT>& seed_nodes)  {
		RelevantFeatureSet features;
		
		std::unordered_set<NodePT> all_visited;
				
		for (NodePT node:seed_nodes) {
			// We ignore s0
			while (node->has_parent()) {
				// If the node has already been processed, no need to do it again, nor to process the parents,
				// which will necessarily also have been processed.
				auto res = all_visited.insert(node);
				if (!res.second) break;
				
				const StateT& state = node->state;
				const auto feature_valuation = _featureset.evaluate(node->state);
				
				for (unsigned feat_idx = 0; feat_idx < feature_valuation.size(); ++feat_idx) {
					features.insert(std::make_pair(feat_idx, feature_valuation[feat_idx]));
				}
				node = node->parent;
			}
		}
		return features;
	}	

	//There is a set of no good atoms per each plan pi (per each goal atom)
	void compute_no_good_atoms() {
	  std::cout << "Computing relevant no good atoms" << std::endl;
	  //const AtomIndex& index = Problem::getInstance().get_tuple_index();
	  const ProblemInfo& info = ProblemInfo::getInstance();

	  //Compute the set of relevant no good atoms for each goal atom
	  for(auto& plan: _optimal_paths)
	    if(plan != nullptr)
	      flag_relevant_no_good_atoms(plan, _relevant_no_good_atoms);
	  
	  
	  std::cout << "Relevant no good atoms before holding(o) subgoals: " << _relevant_no_good_atoms.size() << std::endl;
	  
	  unsigned curr_size = 0;
	  
	  do {
	    
	    curr_size = _relevant_no_good_atoms.size();

	    const std::vector<ObjectIdx> all_objects = info.getTypeObjects("object_id");
	    std::vector<bool> processed(all_objects.size(), false); // This will tell us for each object whether the tuple holding(o) has already been processed.
	    for (unsigned i = 0; i < all_objects.size(); ++i) {
		  ObjectIdx obj = all_objects[i];
		  if (processed[i]) 
		    continue; // No need to process twice the same holding(o) subgoal!
					  
		  VariableIdx confo_var = info.getVariableId("confo(" + info.deduceObjectName(obj, "object_id") + ")"); // TODO This should be precomputed
		  ObjectIdx confo = _init.getValue(confo_var);
		  Atom atom(confo_var, confo);
		  if(_relevant_no_good_atoms.find(atom) == _relevant_no_good_atoms.end())
		    continue;
		  AtomIdx holding_o = _obj_to_holding_tuple_idx.at(obj); // the tuple "holding(o)"

		  NodePT& node = _tuple_to_node.at(holding_o);
		  if(node != nullptr) {//We have a goal node for atom holding(o)
		    flag_relevant_no_good_atoms(node, _relevant_no_good_atoms);//Node contains the plan from s0 to holding(o)
		     _subgoals_path.push_back(node);
		  }
		  else
		    std::cout << "There is no a plan for " << atom << std::endl; //throw std::runtime_error("Simulation max_width too high");
		  
		  processed[i] = true; //we mark this object as processed
					  
	    }
	  } while(_relevant_no_good_atoms.size() > curr_size);
	  
	  std::cout << "Relevant no good atoms after holding(o) subgoals: " << _relevant_no_good_atoms.size() << std::endl;
	  
	  const ExternalI& external = info.get_external();
	  external.load_C(_relevant_no_good_atoms);

	}
	
	
		
	void flag_relevant_no_good_atoms(NodePT node, NoGoodAtomsSet& offending) {
		const ProblemInfo& info = ProblemInfo::getInstance();
		const ExternalI& external = info.get_external();
		const auto& ground_actions = this->_model.getTask().getGroundActions();
		assert(ground_actions.size());

		VariableIdx v_confb = info.getVariableId("confb(rob)");//confb(rob) variable
		VariableIdx v_traja = info.getVariableId("traj(rob)");//traj(rob) variable
		VariableIdx v_holding = info.getVariableId("holding()");//holding variable
		VariableIdx v_confa = info.getVariableId("confa(rob)");//holding variable

		
		while (node->has_parent()) {
		  
			const StateT& state = node->state;
			const GroundAction* action = Problem::getInstance().getGroundActions()[node->action];
			
			if (action->getName() == "transition_arm") {
			  
			  
				ObjectIdx o_confb = state.getValue(v_confb);//Base conf
				ObjectIdx o_confa = state.getValue(v_confa);//Object being held
				ObjectIdx o_traj_arm = state.getValue(v_traja);//trajectory
				ObjectIdx o_held = state.getValue(v_holding);//Object being held
				
				
				//Holding object
				std::string obj_h_name = info.deduceObjectName(o_held, "nullable_object_id");
				VariableIdx idx_gtype_h = info.getVariableId("gtype("+obj_h_name+")");
				auto gtype_o_held = state.getValue(idx_gtype_h);
				
				ObjectIdx gtype_obj = info.getObjectId("g1");
				
				//std::cout << "< " << info.deduceObjectName(o_confb,"conf_base") << ", " << info.deduceObjectName(o_traj_arm,"trajectory") << 
				//", " << info.deduceObjectName(o_held, "object_id") << ", " << info.deduceObjectName(gtype_o_held, "geometry_type") << ", " << info.deduceObjectName(gtype_obj, "geometry_type") << " >" << std::endl;
				auto v_off = external.get_offending_configurations(o_confb, o_traj_arm, o_held, gtype_o_held, gtype_obj);//std::vector<ObjectIdx>
				
				for(ObjectIdx obj: info.getTypeObjects("object_id")) {
				  
				  bool is_graspable = external.graspable(state, o_confb, o_confa, obj);
				  if(is_graspable)
				    continue;
				  
				   VariableIdx confo = _all_objects_conf[obj];
				   ObjectIdx obj_conf = _init.getValue(confo);
				   if(std::find(v_off.begin(), v_off.end(), obj_conf) != v_off.end() )
				     for(auto& bad_conf: v_off)
				       offending.insert(Atom(confo, bad_conf));
				}
				//}

			}
			
		    node = node->parent;
		}
	}
	
	
	
	void report_simulation_stats(float simt0) {
		_stats.simulation();
		_stats.sim_add_time(aptk::time_used() - simt0);
		_stats.sim_add_expanded_nodes(_w1_nodes_expanded+_w2_nodes_expanded);
		_stats.sim_add_generated_nodes(_w1_nodes_generated+_w2_nodes_generated+_w_gt2_nodes_generated);
		_stats.reachable_subgoals( _model.num_subgoals() - _unreached.size());
	}
	
	std::vector<bool> compute_R_all() {
		const AtomIndex& index = Problem::getInstance().get_tuple_index();
		_stats.r_type(1);
		std::vector<bool> all(index.size(), false);
		for (unsigned i = 0; i < all.size(); ++i) {
			const Atom& atom = index.to_atom(i);
			if (atom.getValue()!=0) all[i] = true;
		}
		LPT_INFO("cout", "Simulation - Computed R_All set with " << std::count(all.cbegin(), all.cend(), true) << " atoms");
		return all;
	}
	
	std::vector<bool> compute_R(const StateT& seed, RelevantFeatureSet& F_G) {
		if (_config._force_R_all) {
			if (_verbose) LPT_INFO("cout", "Simulation - R=R[All] is the user-preferred option");	
			return compute_R_all();
		}
		
		if (_config._r_g_prime) {
			if (_verbose) LPT_INFO("cout", "Simulation - R=R'_G is the user-preferred option");	
			return compute_R_g_prime(seed);
		}		
		
		if (_config._force_adaptive_run) {
			throw std::runtime_error("DON'T USE THIS! Use R'_G instead");
			return compute_adaptive_R(seed);
		} else if (_config._max_width == 1){
			return compute_plain_R1(seed);
		} else if (_config._max_width == 2){
			return compute_plain_RG2(seed, F_G);
		} else {
			throw std::runtime_error("Simulation max_width too high");
		}
	}
	
	std::vector<bool> compute_plain_RG2(const StateT& seed, RelevantFeatureSet& F_G) {
		assert(_config._max_width == 2);
		
		_config._complete = false;
		float simt0 = aptk::time_used();
  		run(seed, _config._max_width);
		report_simulation_stats(simt0);
		LPT_INFO("cout", "Simulation - IW(" << _config._max_width << ") run reached " << _model.num_subgoals() - _unreached.size() << " goals");
		compute_no_good_atoms();
		std::vector<bool> R_G = extract_R_G_relaxed(true, F_G);
		extend_R_G(R_G);
		return R_G;
	
		//return extract_R_G(true);
		//return extract_R_G_relaxed(true);
	  
	}
	
	
	void extend_R_G(std::vector<bool>& R_G) {
	  
	  const AtomIndex& index = Problem::getInstance().get_tuple_index();
	  const ProblemInfo& info = ProblemInfo::getInstance();

	  
		std::cout << "Size: " << _subgoals_path.size() << std::endl;
		std::vector<bool> subgoals_R_G = mark_all_atoms_in_path_to_subgoal(_subgoals_path);
		unsigned subgoals_R_G_size = std::count(subgoals_R_G.begin(), subgoals_R_G.end(), true);

		
		/*if (_verbose) {
			LPT_INFO("cout", "Simulation - Extended |R_G[" << _config._max_width << "]| = " << subgoals_R_G_size << " (computed from " << _subgoals_path.size() << " subgoal-reaching nodes)");
			if (subgoals_R_G_size) {
				LPT_INFO("cout", "Simulation - R_G:");
				std::cout << "\t\t";
				for (unsigned i = 0; i < subgoals_R_G.size(); ++i) {
					if (subgoals_R_G[i]) std::cout << index.to_atom(i) << ", ";
				}
				std::cout << std::endl;
			}
		}*/
		
		
		for(unsigned i = 0; i < subgoals_R_G.size(); ++i)
		  if(subgoals_R_G[i] == true)
		    R_G[i] = true;

		unsigned R_G_size = std::count(R_G.begin(), R_G.end(), true);
		
		/*if (_verbose) {
			LPT_INFO("cout", "Simulation - Total |R_G[" << _config._max_width << "]| = " << R_G_size << " (computed from " << _subgoals_path.size() + _optimal_paths.size() << " subgoal-reaching nodes)");
			if (R_G_size) {
				LPT_INFO("cout", "Simulation - R_G:");
				std::cout << "\t\t";
				for (unsigned i = 0; i < R_G.size(); ++i) {
					if (R_G[i]) std::cout << index.to_atom(i) << ", ";
				}
				std::cout << std::endl;
			}
		}*/
		
		
		//return R_G;
	  
	}
	
	
	
	
	std::vector<bool> compute_plain_R1(const StateT& seed) {
		assert(_config._max_width == 1);
		_config._complete = false;
		
		float simt0 = aptk::time_used();
		  std::cout << "Running IW(1)" << std::endl;

  		run(seed, _config._max_width);
		compute_no_good_atoms();
		report_simulation_stats(simt0);
		std::vector<bool> R_G;
		
		
		
		if (_config._goal_directed && _unreached.size() == 0) {
			LPT_INFO("cout", "Simulation - IW(" << _config._max_width << ") reached all subgoals, computing R_G[" << _config._max_width << "]");
			R_G = extract_R_G_1();
		}
		
		// Else, compute the goal-unaware version of R containing all atoms seen during the IW run
		else R_G = extract_R_1();
		
		extend_R_G(R_G);
		
		return R_G;
	}
	
	std::vector<bool> extract_R_1() {
		LPT_INFO("cout", "Simulation - IW(" << _config._max_width << ") run reached " << _model.num_subgoals() - _unreached.size() << " goals");
		if (_verbose) {
			unsigned c = std::count(_seen_w1_atoms.begin(), _seen_w1_atoms.end(), true);
			LPT_INFO("cout", "Simulation - |R[1]| = " << c);
			_stats.relevant_atoms(c);
		}
		return _seen_w1_atoms;		
	}
	
	std::vector<bool> compute_R_g_prime(const StateT& seed) {
		_config._complete = false;
	
		
		float simt0 = aptk::time_used();
  		run(seed, 1);
		compute_no_good_atoms();
		report_simulation_stats(simt0);
		
		if (_unreached.size() == 0) {
			std::vector<NodePT> seed_nodes = extract_seed_nodes();
						
			std::vector<bool> R_G = mark_all_atoms_in_path_to_subgoal(seed_nodes);
			unsigned R_G_size = std::count(R_G.begin(), R_G.end(), true);
			LPT_INFO("cout", "Simulation - IW(1) run reached all goals");
			LPT_INFO("cout", "Simulation - |R_G'[1]| = " << R_G_size << " (computed from " << seed_nodes.size() << " subgoal-reaching nodes)");
			_stats.relevant_atoms(R_G_size);
			return R_G;
			
		}
		
		LPT_INFO("cout", "Simulation - IW(1) run did not reach all goals, throwing IW(2) simulation");

		
		if (_config._gr_actions_cutoff < std::numeric_limits<unsigned>::max()) {
			unsigned num_actions = Problem::getInstance().getGroundActions().size();
			if (num_actions > _config._gr_actions_cutoff) { // Too many actions to compute IW(º2)
				LPT_INFO("cout", "Simulation - Number of actions (" << num_actions << " > " << _config._gr_actions_cutoff << ") considered too high to run IW(2).");
				return compute_R_all();
			} else {
					LPT_INFO("cout", "Simulation - Number of actions (" << num_actions << " <= " << _config._gr_actions_cutoff << ") considered low enough to run IW(2).");
			}
		}
			
		reset();
		_relevant_no_good_atoms.clear();
		run(seed, 2);
		compute_no_good_atoms();
		report_simulation_stats(simt0);
		_stats.reachable_subgoals( _model.num_subgoals() - _unreached.size());
		
		if (_unreached.size() == 0) {
			std::vector<NodePT> seed_nodes = extract_seed_nodes();
			std::vector<bool> R_G = mark_all_atoms_in_path_to_subgoal(seed_nodes);
			unsigned R_G_size = std::count(R_G.begin(), R_G.end(), true);
			LPT_INFO("cout", "Simulation - IW(2) run reached all goals");
			LPT_INFO("cout", "Simulation - |R_G'[2]| = " << R_G_size << " (computed from " << seed_nodes.size() << " subgoal-reaching nodes)");
			_stats.relevant_atoms(R_G_size);
			return R_G;
		}
		
		LPT_INFO("cout", "Simulation - IW(2) run did not reach all goals, falling back to R=R_all");
		return compute_R_all();
	}	
	
	
	//Not reported in the IJCAI paper
	std::vector<bool> compute_adaptive_R(const StateT& seed) {
		const AtomIndex& index = Problem::getInstance().get_tuple_index();
		_config._complete = false;
	
		
		float simt0 = aptk::time_used();
  		run(seed, 1);
		report_simulation_stats(simt0);
		
		if (_unreached.size() == 0) {
			// If a single IW[1] run reaches all subgoals, we return R=emptyset
			LPT_INFO("cout", "Simulation - IW(1) run reached all goals, thus R={}");
			_stats.r_type(0);
			return std::vector<bool>(index.size(), false);
		} else {
			LPT_INFO("cout", "Simulation - IW(1) run did not reach all goals, throwing IW(2) simulation");
		}
		
		// Otherwise, run IW(2)
		reset();
		run(seed, 2);
		report_simulation_stats(simt0);
		_stats.reachable_subgoals( _model.num_subgoals() - _unreached.size());
		
		return extract_R_G(true);
	}	
	
	
	std::vector<bool> extract_R_G_relaxed(bool r_all_fallback, RelevantFeatureSet& F_G) {
	  	const AtomIndex& index = Problem::getInstance().get_tuple_index();
		const ProblemInfo& info = ProblemInfo::getInstance();

		if (r_all_fallback) {
			unsigned num_subgoals = _model.num_subgoals();
			unsigned initially_reached = std::count(_in_seed.begin(), _in_seed.end(), true);
			unsigned reached_by_simulation = num_subgoals - _unreached.size() - initially_reached;
			if (_verbose) LPT_INFO("cout", "Simulation - " << reached_by_simulation << " subgoals were newly reached by the simulation.");
			bool decide_r_all = (reached_by_simulation < (0.5*num_subgoals));
			decide_r_all = _unreached.size() != 0; // XXX Use R_All is any non-reached
			if (decide_r_all) {
				if (_verbose) LPT_INFO("cout", "Simulation - Falling back to R=R[All]");	
				_stats.r_type(1);
				return compute_R_all();
			} else {
				if (_verbose) LPT_INFO("cout", "Simulation - Computing R_G");
			}
		}
		
		
		std::vector<NodePT> seed_nodes = extract_seed_nodes();
 		std::vector<bool> R_G = mark_all_atoms_in_path_to_subgoal(seed_nodes);
		F_G = mark_all_features_in_path_to_subgoal(seed_nodes);

		unsigned R_G_size = std::count(R_G.begin(), R_G.end(), true);
		/*if (_verbose) {
			LPT_INFO("cout", "Simulation - |R_G[" << _config._max_width << "]| = " << R_G_size << " (computed from " << seed_nodes.size() << " subgoal-reaching nodes)");
			if (R_G_size) {
				LPT_INFO("cout", "Simulation - R_G:");
				std::cout << "\t\t";
				for (unsigned i = 0; i < R_G.size(); ++i) {
					if (R_G[i]) std::cout << index.to_atom(i) << ", ";
				}
				std::cout << std::endl;
			}
		}*/
		_stats.relevant_atoms(R_G_size);
		_stats.r_type(2);
		
		return R_G;
	}
	
	//! Extracts the goal-oriented set of relevant atoms after a simulation run
	std::vector<bool> extract_R_G(bool r_all_fallback) {
		throw std::runtime_error("DON'T USE THIS");
		
		const AtomIndex& index = Problem::getInstance().get_tuple_index();

		
		if (r_all_fallback) {
			unsigned num_subgoals = _model.num_subgoals();
			unsigned initially_reached = std::count(_in_seed.begin(), _in_seed.end(), true);
			unsigned reached_by_simulation = num_subgoals - _unreached.size() - initially_reached;
			if (_verbose) LPT_INFO("cout", "Simulation - " << reached_by_simulation << " subgoals were newly reached by the simulation.");
			bool decide_r_all = (reached_by_simulation < (0.5*num_subgoals));
			decide_r_all = _unreached.size() != 0; // XXX Use R_All is any non-reached
			if (decide_r_all) {
				if (_verbose) LPT_INFO("cout", "Simulation - Falling back to R=R[All]");	
				_stats.r_type(1);
				return compute_R_all();
			} else {
				if (_verbose) LPT_INFO("cout", "Simulation - Computing R_G");
			}
		}
		
		
		std::vector<NodePT> seed_nodes = extract_seed_nodes();
		std::vector<bool> R_G(index.size(), false);
		mark_atoms_in_path_to_subgoal(seed_nodes, R_G);
		
		
		unsigned R_G_size = std::count(R_G.begin(), R_G.end(), true);
		if (_verbose) {
			LPT_INFO("cout", "Simulation - |R_G[" << _config._max_width << "]| = " << R_G_size << " (computed from " << seed_nodes.size() << " subgoal-reaching nodes)");
			if (R_G_size) {
				LPT_INFO("cout", "Simulation - R_G:");
				std::cout << "\t\t";
				for (unsigned i = 0; i < R_G.size(); ++i) {
					if (R_G[i]) std::cout << index.to_atom(i) << ", ";
				}
				std::cout << std::endl;
			}
		}
		_stats.relevant_atoms(R_G_size);
		_stats.r_type(2);
		
		return R_G;
		
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
	
	std::vector<bool> extract_R_G_1() {
		std::vector<NodePT> seed_nodes = extract_seed_nodes();
		std::vector<bool> R_G = mark_all_atoms_in_path_to_subgoal(seed_nodes);
		
		unsigned R_G_size = std::count(R_G.begin(), R_G.end(), true);
		if (_verbose) {
			LPT_INFO("cout", "Simulation - |R_G[" << _config._max_width << "]| = " << R_G_size << " (computed from " << seed_nodes.size() << " subgoal-reaching nodes)");
		}
		_stats.relevant_atoms(R_G_size);
		_stats.r_type(2);
		
		return R_G;
	}	
	

	
	bool run(const StateT& seed, unsigned max_width) {
		/*if (_verbose) LPT_INFO("cout", "Simulation - Starting IW Simulation");
		
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

				for (const auto& a : _model.applicable_actions(current->state)) {
					StateT s_a = _model.next( current->state, a );
					NodePT successor = std::make_shared<NodeT>(std::move(s_a), a, current, _generated++);
					
					unsigned char novelty = _evaluator.evaluate(*successor);
					update_novelty_counters_on_generation(novelty);
					
					// LPT_INFO("cout", "Simulation - Node generated: " << *successor);
					
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
		return false;*/
		return run(seed);
	}
	
	
	bool run(const StateT& seed) {
		if (_verbose) LPT_INFO("cout", "Starting IW Simulation");
		
		NodePT n = std::make_shared<NodeT>(seed, _generated++);
		mark_seed_subgoals(n);
		
		auto nov =_evaluator.evaluate(*n);
		_unused(nov);
		assert(nov==1);
		++_w1_nodes_expanded; 
		++_w1_nodes_generated;
// 		LPT_DEBUG("cout", "Simulation - Seed node: " << *n);
		
		OpenListT open;

		open.insert(n);

		// Note that we don't used any closed list / duplicate detection of any kind, but let the novelty engine take care of that
		while (!open.empty()) {
			NodePT current = open.next( );

			if (current->_w == 1) 
			  ++_w1_nodes_expanded;
			else if (current->_w == 2) 
			  ++_w2_nodes_expanded;

			for (const auto& a : _model.applicable_actions(current->state)) {
				StateT s_a = _model.next( current->state, a );
				NodePT successor = std::make_shared<NodeT>( std::move(s_a), a, current, _generated++ );
				
				unsigned novelty = _evaluator.evaluate(*successor);
// 				if (novelty == 1) {
// 					_w1_nodes.push_back(successor);
// 				}
				
//   				LPT_INFO("cout", "Simulation - Node generated: " << *successor);
				
				if (process_node(successor)) {  // i.e. all subgoals have been reached before reaching the bound
					report("All subgoals reached");	
					return true;
				}
				
				if (novelty <= _config._max_width) {
					open.insert(successor);
					
					if (novelty==1) save_w1_atoms(successor);
					
					assert(novelty == 1 || novelty == 2);
					
					if (novelty==1) 
					  ++_w1_nodes_generated;
					else  
					  ++_w2_nodes_generated;
				} else {
					++_w_gt2_nodes_generated;
				}
			}
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
		LPT_INFO("cout", "Simulation - Result: " << result);
		LPT_INFO("cout", "Simulation - Num reached subgoals: " << (_model.num_subgoals() - _unreached.size()) << " / " << _model.num_subgoals());
		LPT_INFO("cout", "Simulation - Expanded nodes with w=1 " << _w1_nodes_expanded);
		LPT_INFO("cout", "Simulation - Expanded nodes with w=2 " << _w2_nodes_expanded);		
		LPT_INFO("cout", "Simulation - Generated nodes with w=1 " << _w1_nodes_generated);
		LPT_INFO("cout", "Simulation - Generated nodes with w=2 " << _w2_nodes_generated);
		LPT_INFO("cout", "Simulation - Generated nodes with w>2 " << _w_gt2_nodes_generated);
	}

protected:

	//! Returns true iff all goal atoms have been reached in the IW search
	bool process_node(NodePT& node) {
		//if (_config._complete) 
		  //return process_node_complete(node);

		const StateT& state = node->state;
		
		const AtomIndex& index = Problem::getInstance().get_tuple_index();
		
		for (unsigned i = 0; i < state.numAtoms(); ++i) {
			AtomIdx idx = index.to_index(i, state.getValue(i));
			if (_tuple_to_node[idx] == nullptr) {
				_tuple_to_node[idx] = node;
			}
		}
		

		// We iterate through the indexes of all those goal atoms that have not yet been reached in the IW search
		// to check if the current node satisfies any of them - and if it does, we mark it appropriately.
		for (auto it = _unreached.begin(); it != _unreached.end(); ) {
			unsigned subgoal_idx = *it;

			if (_model.goal(state, subgoal_idx)) {
// 				node->satisfies_subgoal = true;
// 				_all_paths[subgoal_idx].push_back(node);
				if (!_optimal_paths[subgoal_idx]) 
				  _optimal_paths[subgoal_idx] = node;
				it = _unreached.erase(it);
			} else {
				++it;
			}
		}
		// As soon as all nodes have been processed, we return true so that we can stop the search
		return _unreached.empty();
		//return false;
	}
	
	// store all of the atoms in the node
	void save_w1_atoms(NodePT& node) {
		const AtomIndex& index = Problem::getInstance().get_tuple_index();
		assert(_seen_w1_atoms.size() == index.size());
		const StateT& state = node->state;
		for (unsigned var = 0; var < state.numAtoms(); ++var) {
			ObjectIdx val = state.getValue(var);
			if (val != 0) {
				_seen_w1_atoms[index.to_index(var, val)] = true;
			}			
		}		
	}
	
	
	
	
	//! Returns true iff all goal atoms have been reached in the IW search
	bool process_node_complete(NodePT& node) {
		const StateT& state = node->state;
		for (unsigned i = 0; i < _model.num_subgoals(); ++i) {
			if (!_in_seed[i] && _model.goal(state, i)) {
// 				node->satisfies_subgoal = true;
				if (!_optimal_paths[i]) 
				  _optimal_paths[i] = node;
				_unreached.erase(i);
			}
		}
 		return _unreached.empty();
		//return false; // return false so we don't interrupt the processing
	}
	
	void mark_seed_subgoals(const NodePT& node) {
		const AtomIndex& index = Problem::getInstance().get_tuple_index();
		std::vector<bool> _(_model.num_subgoals(), false);
		_in_seed.swap(_);
		_seen_w1_atoms = std::vector<bool>(index.size(), false);
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
