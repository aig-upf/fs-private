

#pragma once

template <typename NodeT,
          typename StateModel,
          typename NoveltyEvaluatorT,
		  typename FeatureSetT,
          typename OpenListT = lapkt::SearchableQueue<NodeT>,
          typename ClosedListT = aptk::StlUnorderedMapClosedList<NodeT>
>
class IWRun : public lapkt::GenericSearch<NodeT, OpenListT, ClosedListT, StateModel> {

	//! Retrieve the set of atoms which are relevant to reach at least one of the subgoals
	//! Additionally, leaves in the class attribute '_visited' pointers to all those nodes which
	//! are on at least one of the paths from the seed state to one of the nodes that satisfies a subgoal.
	RelevantAtomSet retrieve_relevant_atoms(const StateT& seed, unsigned& reachable) {
		const AtomIndex& atomidx = this->_model.getTask().get_tuple_index();
		RelevantAtomSet atomset(&atomidx);

		// atomset.mark(seed, RelevantAtomSet::STATUS::UNREACHED); // This is not necessary, since all these atoms will be made true by the "root" state of the simulation

		_visited.clear();

		// Iterate through all the subgoals that have been reached, and rebuild the path from the seed state to reach them
		// adding all atoms encountered in the path to the RelevantAtomSet as "relevant but unreached"
		for (unsigned subgoal_idx = 0; subgoal_idx < _all_paths.size(); ++subgoal_idx) {
			const std::vector<NodePT>& paths = _all_paths[subgoal_idx];
			
			if (_in_seed[subgoal_idx] || !paths.empty()) {
				++reachable;
			}
			
			for (const NodePT& node:paths) {
				process_path_node(node, atomset);
			}
		}

		return atomset;
	}
	
	void process_path_node(NodePT node, RelevantAtomSet& atomset) {
		// Traverse from the solution node to the root node, adding all atoms on the way
		// if (node->has_parent()) node = node->parent; // (Don't) skip the last node
		while (node->has_parent()) {
			// If the node has already been processed, no need to do it again, nor to process the parents,
			// which will necessarily also have been processed.
			if (_visited.find(node) != _visited.end()) break;
			
			// Mark all the atoms in the state as "yet to be reached"
			atomset.mark(node->state, &(node->parent->state), RelevantAtomSet::STATUS::UNREACHED, _config._mark_negative, false);
			_visited.insert(node);
			node = node->parent;
		}
		
		_visited.insert(node); // Insert the root anyway to mark it as a relevant node
	}
	
	
	AtomIdx select_atom(const std::unordered_set<NodePT>& nodes) const {
		std::unordered_map<AtomIdx, unsigned> counts;
		
		for (const auto& node:nodes) {
			for (const AtomIdx atom:node->_relevant_atoms) {
				counts[atom]++;
			}
		}
		
// 		for (const auto& c:counts) LPT_INFO("cout", "Atom " << c.first << " count: " << c.second);
		
		using T = typename std::unordered_map<AtomIdx, unsigned>::value_type;
		return std::max_element(counts.begin(), counts.end(), [](T a, T b){ return a.second < b.second; })->first;
	}
	
	// TODO Optimize this very inefficient prototype
	std::unordered_set<AtomIdx> compute_hitting_set(std::unordered_set<NodePT>& nodes) const {
		std::unordered_set<AtomIdx> hs;
		while (!nodes.empty()) {
			AtomIdx selected = select_atom(nodes);
			
// 			LPT_INFO("cout", "IW Simulation - Selected atom: " << selected << ": " << index.to_atom(selected));
			
			for (auto it = nodes.begin(); it != nodes.end();) {
				const std::unordered_set<unsigned>& rset = (*it)->_relevant_atoms;
				if (rset.find(selected) != rset.end()) {
					it = nodes.erase(it);
				} else {
					++it;
				}
			}
			hs.insert(selected);
		}
		
		return hs;
		
	}
	
	std::unordered_set<AtomIdx> compute_union(std::unordered_set<NodePT>& nodes) const {
		std::unordered_set<AtomIdx> set_union;
		for (const auto& node:nodes) {
			set_union.insert(node->_relevant_atoms.begin(), node->_relevant_atoms.end());
		}
		return set_union;
	}
	

	
	std::vector<bool> compute_R_union_Rs(const StateT& seed) {
		_config._max_width = 2;
// 		_config._bound = -1; // No bound
		std::vector<NodePT> w1_seed_nodes;
		auto atoms = compute_R(seed, w1_seed_nodes);
		
		const AtomIndex& index = Problem::getInstance().get_tuple_index();
		std::vector<bool> res(index.size(), false);
		for (AtomIdx atom:atoms) {
			res[atom] = true;
		}
		return res;
	}
	
	std::unordered_set<NodePT> compute_relevant_w2_nodes() const {
		std::unordered_set<NodePT> all_visited;
		std::unordered_set<NodePT> w2_nodes;
		for (NodePT node:_w1_nodes) {
			process_w1_node(node, w2_nodes, all_visited);
		}
		return w2_nodes;
	}
	
	void process_w1_node(NodePT node, std::unordered_set<NodePT>& w2_nodes, std::unordered_set<NodePT>& all_visited) const {
		// Traverse from the solution node to the root node
		
		NodePT root = node;
		// We ignore s0
		while (node->has_parent()) {
			// If the node has already been processed, no need to do it again, nor to process the parents,
			// which will necessarily also have been processed.
			auto res = all_visited.insert(node);
			if (!res.second) break;
			
			if (node->_w == 2) {
				w2_nodes.insert(node);
			}
			node = node->parent;
		}
	}	
	
};

template <typename StateModelT, typename NoveltyIndexerT, typename FeatureSetT, typename NoveltyEvaluatorT>
class SBFWSHeuristic {

/*
void dump_simulation_nodes(NodePT& node) {
	_heuristic.run_simulation(*node);
	const auto& simulation_nodes = _heuristic.get_last_simulation_nodes();
	auto search_nodes = convert_simulation_nodes(node, simulation_nodes);
	// std::cout << "Got " << simulation_nodes.size() << " simulation nodes, of which " << search_nodes.size() << " reused" << std::endl;
	for (const auto& n:search_nodes) {
		//create_node(n);
		_q1.insert(n);
		_stats.simulation_node_reused();
		// std::cout << "Simulation node reused: " << *n << std::endl;
	}
}
*/



	//! Return a newly-computed set of atoms which are relevant to reach the goal from the given state, with
	//! all those atoms marked as "unreached", and the rest as irrelevant.
	//! If 'log_stats' is true, the stats of this simulation will be logged in the '_stats' atribute.
	/*
	RelevantAtomSet compute_relevant_simulation(const State& state, unsigned& reachable) {
		reachable = 0;

		_simulator = std::unique_ptr<SimulationT>(new SimulationT(_model, _featureset, _simconfig));

		//BFWSStats stats;
		//StatsObserver<IWNodeT, BFWSStats> st_obs(stats, false);
		//iw->subscribe(st_obs);

		_simulator->run(state);

		RelevantAtomSet relevant = _simulator->retrieve_relevant_atoms(state, reachable);

		//LPT_INFO("cout", "IW Simulation: Node expansions: " << stats.expanded());
		//LPT_INFO("cout", "IW Simulation: Node generations: " << stats.generated());

		return relevant;
	}

	void compute_relevant(const State& state, bool log_stats, RelevantAtomSet& atomset) {
		
		unsigned reachable = 0, max_reachable = _model.num_subgoals();
		_unused(max_reachable);
		if (_aptk_rpg) {
			compute_relevant_aptk_hff(state, atomset);
		} else if (_use_simulation_nodes) {
			// Leave the relevant atom set empty
			assert(false); // This needs to be rethought
// 			compute_relevant_simulation(state, reachable);
		} else {
			compute_relevant_simulation(state, reachable, atomset);
		}

		LPT_EDEBUG("simulation-relevant", "Computing R(s) from state: " << std::endl << state << std::endl);
		LPT_EDEBUG("simulation-relevant", relevant.num_unreached() << " relevant atoms (" << reachable << "/" << max_reachable << " reachable subgoals): " << print::relevant_atomset(atomset) << std::endl << std::endl);

		if (log_stats) {
			_stats.set_initial_reachable_subgoals(reachable);
			_stats.set_initial_relevant_atoms(atomset.num_unreached());
		}
		_stats.reachable_subgoals(reachable);
		_stats.relevant_atoms(atomset.num_unreached());
		_stats.simulation();

	}

	void compute_relevant_aptk_hff(const State& state, RelevantAtomSet& atomset) {
		assert(_aptk_rpg);
		const AtomIndex& atomidx = _problem.get_tuple_index();
		return _aptk_rpg->compute_r_ff(state, atomidx);
	}
	*/

		/*
	template <typename NodeT>
	void run_simulation(NodeT& node) {
// 		assert(_use_simulation_nodes);
		if (node._simulated) return;
		
		node._simulated = true;
		
		unsigned reachable = 0, max_reachable = _model.num_subgoals();
		_unused(max_reachable);
		// Leave the relevant atom set empty
		compute_relevant_simulation(node.state, reachable);

		LPT_EDEBUG("simulation-relevant", "Running simulation from state: " << std::endl << node.state << std::endl);
		LPT_EDEBUG("simulation-relevant", " " << reachable << "/" << max_reachable << " reachable subgoals" << std::endl << std::endl);

		if (!node.has_parent()) {
			_stats.set_initial_reachable_subgoals(reachable);
		}
		_stats.reachable_subgoals(reachable);
		_stats.simulation();
	}
	*/

		template <typename StateT>
	std::vector<bool> compute_R_union_Rs(const StateT& state) {
		throw std::runtime_error("Revise");
// 		_stats.simulation();
// 		SimulationT simulator(_model, _featureset, _simconfig);
// 		return simulator.compute_R_union_Rs(state);
	}
	
const std::unordered_set<IWNodePT>& get_last_simulation_nodes() const { return _simulator->get_relevant_nodes(); }


	~SBFWSHeuristic() {
		for (auto& p:_wg_half_novelty_evaluators) delete p.second;
	};
	NoveltyEvaluatorMapT _wg_half_novelty_evaluators; // 1.5 nov evaluators

	template <typename NodeT>
	bool evaluate_wg1_5(NodeT& node, const std::vector<AtomIdx>& special) {
		// LPT_DEBUG("cout", "Let's compute whether node has novelty 1,5: " << node);

		assert(node.w_g != Novelty::Unknown);
		unsigned type = node.unachieved_subgoals;
		bool has_parent = node.has_parent();
		unsigned ptype = has_parent ? node.parent->unachieved_subgoals : 0; // If the node has no parent, this value doesn't matter.

		// A somewhat special routine for dealing with 1.5 computations
		bool res;
		NoveltyEvaluatorT* evaluator = fetch_evaluator(_wg_half_novelty_evaluators, type);
		
		if (has_parent && type == ptype) {
			res = evaluator->evaluate_1_5(_featureset.evaluate(node.state), _featureset.evaluate(node.parent->state), special);
		} else {
			res = evaluator->evaluate_1_5(_featureset.evaluate(node.state), special);
		}
		
		if (node.w_g != Novelty::One) {
			node.w_g = res ? Novelty::OneAndAHalf : Novelty::GTOneAndAHalf;
		}
		return res;
	}

};





template <typename StateModelT, typename FeatureSetT, typename NoveltyEvaluatorT>
class SBFWS {
	//! A list with all nodes that have novelty w_{#g}=1.5
// 	UnachievedOpenList _q1half;
	
			/*
		///// 1.5-width QUEUE /////
		// Check whether there are nodes with w_{#g, #r} = 1
		
		if (!_q1half.empty()) {
			LPT_EDEBUG("multiqueue-search", "Checking for open nodes with w_{#g} = 1.5");
			NodePT node = _q1half.next();

			// Greedy 1,5-novelty evaluation
			
// 			if (!node->_processed) {
// 				_stats.wg1_5_node();
// 				process_node(node);
// 			}
			
			
			// Lazy 1,5-novelty evaluation:
			
			bool novel = _heuristic.evaluate_wg1_5(*node, _R);
			if (!node->_processed && novel) {
				_stats.wg1_5_node();
				process_node(node);
			} else if (_novelty_levels == 2) {
				_qrest.insert(node);
			}
			

			// We might have processed one node but found no goal, let's start the loop again in case some node with higher priority was generated
			return true;
		}
		*/
};



//! A RelevantAtomSet contains information about which of the atoms of a problem are relevant for a certain
//! goal, and, among those, which have already been reached and which others have not.
class RelevantAtomSet {
public:
	enum class STATUS : unsigned char {IRRELEVANT, UNREACHED, REACHED};

	//! A RelevantAtomSet is always constructed with all atoms being marked as IRRELEVANT
	RelevantAtomSet(const AtomIndex* atomidx) :
		_atomidx(atomidx), _num_reached(0), _num_unreached(0), _status(atomidx ? atomidx->size() : 0, STATUS::IRRELEVANT)
	{}

	~RelevantAtomSet() = default;
	RelevantAtomSet(const RelevantAtomSet&) = default;
	RelevantAtomSet(RelevantAtomSet&&) = default;
	RelevantAtomSet& operator=(const RelevantAtomSet&) = default;
	RelevantAtomSet& operator=(RelevantAtomSet&&) = default;

	//! Marks all the atoms in the state with the given 'status'.
	//! If 'mark_negative_propositions' is false, predicative atoms of the form X=false are ignored
	//! If 'only_if_relevant' is true, only those atoms that were not deemed _irrelevant_ (i.e. their status was either reached or unreached)
	//! are marked
	void mark(const State& state, const State* parent, STATUS status, bool mark_negative_propositions, bool only_if_relevant) {
		assert(_atomidx);
		const ProblemInfo& info = ProblemInfo::getInstance();
		unsigned n = state.numAtoms();
		for (VariableIdx var = 0; var < n; ++var) {
			ObjectIdx val = state.getValue(var);
			if (!mark_negative_propositions && info.isPredicativeVariable(var) && val==0) continue; // We don't want to mark negative propositions
			if (parent && (val == parent->getValue(var))) continue; // If a parent was provided, we check that the value is new wrt the parent
			mark(_atomidx->to_index(var, val), status, only_if_relevant);
		}
	}

	//! Marks the atom with given index with the given status.
	//! If 'only_if_relevant' is true, then only marks the atom if it was previously
	//! marked as relevant (i.e. its status was _not_ STATUS::IRRELEVANT).
	void mark(AtomIdx idx, STATUS status, bool only_if_relevant) {
		assert(status==STATUS::REACHED || status==STATUS::UNREACHED);
		auto& st = _status[idx];
		if (only_if_relevant && (st == STATUS::IRRELEVANT)) return;

		if (st != status) {
			if (status==STATUS::REACHED) ++_num_reached;
			else if (status==STATUS::UNREACHED) ++_num_unreached;
			
			if (st==STATUS::REACHED) --_num_reached; // The old status was reached, and will not be anymore, so we decrease the counter.
			else if (st==STATUS::UNREACHED) --_num_unreached;
			
			st = status;
		}
	}

	unsigned num_reached() const { return _num_reached; }
	unsigned num_unreached() const { return _num_unreached; }
	
	bool valid() const { return _atomidx != nullptr; }

	friend class print::relevant_atomset;

protected:
	//! A reference to the global atom index
	const AtomIndex* _atomidx;

	//! The total number of reached / unreached atoms
	unsigned _num_reached;
	unsigned _num_unreached;

	//! The status of each atom (indexed by its atom index)
	std::vector<STATUS> _status;
};



















