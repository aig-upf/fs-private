#pragma once

#include <fs_types.hxx>
//#include <heuristics/novelty/features.hxx>
#include <unordered_set>
#include <search/drivers/sbfws/iw_run.hxx>


namespace fs0 { class Config; class Problem; class BasicApplicabilityAnalyzer; }

namespace fs0 { namespace language { namespace fstrips { class Formula; } }}
namespace fs = fs0::language::fstrips;

using OffendingSet = std::unordered_set<fs0::ObjectIdx>;


namespace fs0 { namespace bfws {


class EBFWSPreprocessor {
public:
	static std::vector<OffendingSet> preprocess(const Problem& problem, const Config& config, BasicApplicabilityAnalyzer* analyzer);
	
};

//We can use the class IWRunNode as Node. We don't need to create another one!!!
template <typename NodeT,
          typename StateModel,
          typename NoveltyEvaluatorT,
		  typename FeatureSetT,
          typename OpenListT = lapkt::SearchableQueue<NodeT>,
          typename ClosedListT = apkt::StlUnorderedMapClosedList<NodeT>
>
class AllSolutionsBreadthFirstSearch : public lapkt::GenericSearch<NodeT, OpenListT, ClosedListT, StateModel>
{
public:
	using ActionT = typename StateModel::ActionType;
	using Base = lapkt::GenericSearch<NodeT, OpenListT, ClosedListT, StateModel>;
	using StateT = typename StateModel::StateType;
	using PlanT = typename Base::PlanT;
	using NodePT = typename Base::NodePtr;
	
	using NodeOpenEvent = typename Base::NodeOpenEvent;
	using NodeExpansionEvent = typename Base::NodeExpansionEvent;
	using NodeCreationEvent = typename Base::NodeCreationEvent;

	//! The constructor requires the user of the algorithm to inject both
	//! (1) the state model to be used in the search
	//! (2) the particular open and closed list objects
	AllSolutionsBreadthFirstSearch(const StateModel& model, OpenListT&& open, const std::vector<const fs::AtomicFormula*>& goal, const std::vector<const fs::AtomicFormula*>& state_constraints, bool all_solutions) :
		Base(model, std::move(open), ClosedListT()),
		_info(ProblemInfo::getInstance()),
		_tuple_idx(model.getTask().get_tuple_index()),
		_init(model.init()),
		_all_solutions(all_solutions),
		_goal_atoms(goal), _sc_atoms(state_constraints),
		_optimal_paths(_goal_atoms.size(), nullptr),
		_tuple_to_node(_tuple_idx.size(), nullptr),
		_obj_to_holding_tuple_idx(_info.getNumObjects(), -1)
	{
		VariableIdx holding_v = _info.getVariableId("holding()");
		for (ObjectIdx obj:_info.getTypeObjects("object_id")) {
			TupleIdx t = _tuple_idx.to_index(holding_v, obj); // i.e. the tuple index of the atom holding()=o
			_obj_to_holding_tuple_idx.at(obj) = t;
		}
	
	
	}
	
	virtual ~AllSolutionsBreadthFirstSearch() = default;
	
	// Disallow copy, but allow move
	AllSolutionsBreadthFirstSearch(const AllSolutionsBreadthFirstSearch&) = delete;
	AllSolutionsBreadthFirstSearch(AllSolutionsBreadthFirstSearch&&) = default;
	AllSolutionsBreadthFirstSearch& operator=(const AllSolutionsBreadthFirstSearch&) = delete;
	AllSolutionsBreadthFirstSearch& operator=(AllSolutionsBreadthFirstSearch&&) = default;
	
	bool search(const StateT& s, PlanT& solution) override {
		NodePT n = std::make_shared<NodeT>(s);
		this->notify(NodeCreationEvent(*n));
		this->_open.insert(n);
		
		while ( !this->_open.is_empty() ) {
			NodePT current = this->_open.get_next( );
			this->notify(NodeOpenEvent(*current));
			
			bool goals_reached = process_node(current);
			if (!_all_solutions && goals_reached) {
				break;
			}

			// close the node before the actual expansion so that children which are identical
			// to 'current' get properly discarded
			this->_closed.put(current);
			
			this->notify(NodeExpansionEvent(*current));
			
			for ( const auto& a : this->_model.applicable_actions( current->state ) ) {
				StateT s_a = this->_model.next( current->state, a );
				NodePT successor = std::make_shared<NodeT>( std::move(s_a), a, current );
				
// 				progress_node(successor);
				
				if (this->_closed.check(successor)) continue; // The node has already been closed
				
				this->notify(NodeCreationEvent(*successor));
				if (!this->_open.insert( successor )) {
					LPT_DEBUG("search", std::setw(7) << "PRUNED: " << *successor);
				}
			}
		}
		
		return final_process();
	}
	
protected:
	const ProblemInfo& _info;
	const TupleIndex& _tuple_idx;
	const StateT _init;
	
	//! Whether we want to compute IW up until the end or only until paths to all subgoals
	//! are found _for the first time_
	bool _all_solutions;
	
	//! An index of the nodes with are in the open list at any moment, for faster access
// 	using node_unordered_set = std::unordered_set<NodePT, node_hash<NodePT>, node_equal_to<NodePT>>;
// 	node_unordered_set _solutions;
	
	const std::vector<const fs::AtomicFormula*>& _goal_atoms;
	const std::vector<const fs::AtomicFormula*>& _sc_atoms;
	
	//! _optimal_paths[i] contains the node with a minimum number of state constraint violations
	//! that reaches the goal atom 'i'.
	std::vector<NodePT> _optimal_paths;
	
	//! The node first achieving each problem tuple
	std::vector<NodePT> _tuple_to_node;
	
	//! Object_id o to the index of the tuple "holding()=o"
	std::vector<TupleIdx> _obj_to_holding_tuple_idx;
	
	
// 	void progress_node(NodePT& node) {
// 		const StateT& state = node->state;
// 		for (unsigned sc_atom_idx = 0; sc_atom_idx < _sc_atoms.size(); ++sc_atom_idx) {
// 			const fs::AtomicFormula* atom = _sc_atoms[sc_atom_idx];
// 			if (!atom->interpret(state)) {
// 				// The problem state violates the state constraint atom 'atom'
// 				node->state_constraint_violation(sc_atom_idx);
// 			}
// 		}
// 	}
	
	bool process_node(const NodePT& node) {
		const StateT& state = node->state;
		unsigned num_satisfied = 0;
		
		for (unsigned i = 0; i < state.numAtoms(); ++i) {
			TupleIdx idx = _tuple_idx.to_index(i, state.getValue(i));
			if (_tuple_to_node[idx] == nullptr) {
				_tuple_to_node[idx] = node;
			}
		}
		
		for (unsigned goal_atom_idx = 0; goal_atom_idx < _goal_atoms.size(); ++goal_atom_idx) {
			const fs::AtomicFormula* atom = _goal_atoms[goal_atom_idx];
			NodePT& optimal = _optimal_paths[goal_atom_idx];
			
			if (optimal) continue; // We are glad with the first path we found
			
			if (atom->interpret(state)) { // The state satisfies goal atom with index 'i'
				optimal = node;
				++num_satisfied;
// 				if (!optimal) {
// 					LPT_INFO("cout", "PREPROCESSING: Goal atom '" << *atom << "' reached for the first time");
// 				}
// 				if (!optimal || optimal->num_violations() > node->num_violations()) {
// 					optimal = node;
// 				}
			}
			
// 			if (optimal) {
// 				++num_satisfied;
// 			}
		}
		
		return false; // RUN IW(2) ALWAYS TIL THE END
		return num_satisfied == _goal_atoms.size();
	}
	
	//! Returns true iff all goal atoms have been reached
	bool final_process() {
		for (unsigned goal_atom_idx = 0; goal_atom_idx < _goal_atoms.size(); ++goal_atom_idx) {
			const fs::AtomicFormula* atom = _goal_atoms[goal_atom_idx];
			const NodePT& optimal = _optimal_paths[goal_atom_idx];
			if (!optimal) { // 
				LPT_INFO("cout", "PREPROCESSING: Goal atom '" << *atom << "' is not reachable!");
				return false;
			}
		}
		return true;
	}

public:
	
	//! One offending set per each goal atom
	std::vector<OffendingSet> compute_offending_configurations() {
		std::vector<OffendingSet> offending_0 = compute_goal_offending_configurations();
		
		const std::vector<ObjectIdx> all_objects = _info.getTypeObjects("object_id");
		
		// Now augment this set as follows:
		// Taking each offending configuration, and check whether some object o is in that config in the initial state. If not, continue with the next configuration.
		// If yes, flag as "offending" all the configurations that themselves offend the precomputed trajectory to reach the atom "holding(o)".
		// Continue recursively.
		for (unsigned i = 0; i < offending_0.size(); ++i) {
			OffendingSet& offending = offending_0[i];
			
			unsigned offending0_size = offending.size();
			unsigned cur_size = 0;
		
			std::vector<bool> processed(all_objects.size(), false); // This will tell us for each object whether the tuple holding(o) has already been processed.
			
			do {
				cur_size = offending.size();
				
				for (unsigned j = 0; j < all_objects.size(); ++j) {
					ObjectIdx obj = all_objects[j];
					if (processed[j]) continue; // No need to process twice the same holding(o) subgoal!
					
					VariableIdx confo_var = _info.getVariableId("confo(" + _info.deduceObjectName(obj, "object_id") + ")"); // TODO This should be precomputed
					ObjectIdx confo = _init.getValue(confo_var);
					if (offending.find(confo) == offending.end()) continue;
					
					// Otherwise, object o is initially in an offending configuration
					
					TupleIdx holding_o = _obj_to_holding_tuple_idx.at(obj); // the tuple "holding(o)"
					NodePT& node = _tuple_to_node.at(holding_o);
					flag_offending_configurations(node, offending);
					processed[j] = true;
				}
				
			} while(offending.size() > cur_size);
			
			
			LPT_INFO("cout", "A total of " << offending.size() << " (" << offending0_size << " + " << offending.size() - offending0_size << ") real object configurations found to be offending to goal atom " << *_goal_atoms[i]);
		}
		
		// print_offending_graph(offending_0);
		
		return offending_0;
	}
	
	void print_offending_graph(const std::vector<OffendingSet>& all_offending) {
		
		VariableIdx holding_v = _info.getVariableId("holding()");
		for (ObjectIdx obj:_info.getTypeObjects("object_id")) {
			std::string o1_name = _info.deduceObjectName(obj, "object_id");
			TupleIdx t = _tuple_idx.to_index(holding_v, obj); // i.e. the tuple index of the atom holding()=o
			
			OffendingSet offending;
			NodePT& node = _tuple_to_node.at(t);
			flag_offending_configurations(node, offending);
			
			print_offending_objects("holding(" + o1_name + ")", offending);
		}
		
		LPT_INFO("deadlocks", "");LPT_INFO("deadlocks", "");LPT_INFO("deadlocks", "");LPT_INFO("deadlocks", "");
		
		for (unsigned goal_atom_idx = 0; goal_atom_idx < _goal_atoms.size(); ++goal_atom_idx) {
			const fs::AtomicFormula* atom = _goal_atoms[goal_atom_idx];
			LPT_INFO("deadlocks", "Goal Atom " << *atom << " offended by: ");
			print_offending_objects("", all_offending.at(goal_atom_idx));
		}
		
	}
	
	void print_offending_objects(const std::string& to, const OffendingSet& offending) {
		
		for (ObjectIdx obj2:_info.getTypeObjects("object_id")) {
			std::string o2_name = _info.deduceObjectName(obj2, "object_id");
			VariableIdx confo_var = _info.getVariableId("confo(" + o2_name + ")");
			ObjectIdx confo = _init.getValue(confo_var);
			if (offending.find(confo) != offending.end()) {
				LPT_INFO("deadlocks", to << " - offended by " << o2_name);
			}
		}
	}
};

} } // namespaces

