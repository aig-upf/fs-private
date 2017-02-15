

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

};

template <typename StateModelT, typename NoveltyIndexerT, typename FeatureSetT, typename NoveltyEvaluatorT>
class LazyBFWSHeuristic {

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

	void compute_relevant(const State& state, bool log_stats, LightRelevantAtomSet& atomset) {
		
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

	void compute_relevant_aptk_hff(const State& state, LightRelevantAtomSet& atomset) {
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

const std::unordered_set<IWNodePT>& get_last_simulation_nodes() const { return _simulator->get_relevant_nodes(); }
};