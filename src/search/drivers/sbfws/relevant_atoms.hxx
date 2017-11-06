
#pragma once
#include "iw_run.hxx"


namespace fs0 { class Problem; class L0Heuristic; }
namespace fs0 { namespace bfws { struct SBFWSConfig; class RelevantAtomSet; class BFWSStats; } }
namespace fs0 { namespace hybrid { class L2Norm; }}

    
	
namespace fs0 { namespace bfws {


//! 
template <typename NodeT>
class RelevantAtomsCounterI {
public:
	virtual unsigned count(NodeT& node, BFWSStats& stats) const = 0;
};


//!
template <typename NodeT>
class NullRelevantAtomsCounter : public RelevantAtomsCounterI<NodeT> {
public:
	unsigned count(NodeT& node, BFWSStats& stats) const override { return 0; } 
};


//!
template <typename NodeT>
class L2NormRelevantAtomsCounter : public RelevantAtomsCounterI<NodeT> {
public:
	L2NormRelevantAtomsCounter(const Problem& problem);
	~L2NormRelevantAtomsCounter();
	unsigned count(NodeT& node, BFWSStats& stats) const override;
	
protected:
	hybrid::L2Norm*	_l2_norm;
};


//!
template <typename NodeT>
class L0RelevantAtomsCounter : public RelevantAtomsCounterI<NodeT> {
public:
	L0RelevantAtomsCounter(const Problem& problem);
	~L0RelevantAtomsCounter();
	unsigned count(NodeT& node, BFWSStats& stats) const override;
	
protected:
	L0Heuristic* _l0_heuristic;
};


//!
template <typename NodeT, typename SimulationT, typename NoveltyEvaluatorT>
class SimulationBasedRelevantAtomsCounter : public RelevantAtomsCounterI<NodeT> {
public:
	using FeatureValueT = typename NoveltyEvaluatorT::FeatureValueT;
	
	SimulationBasedRelevantAtomsCounter(const SBFWSConfig& config, const Problem& problem);
	~SimulationBasedRelevantAtomsCounter();

	
	unsigned count(NodeT& node, BFWSStats& stats) const override { 
		return compute_R(node, stats).num_reached();
	} 
	
	
	
	//! Compute the RelevantAtomSet that corresponds to the given node, and from which
	//! the counter #r(node) can be obtained. This implements a lazy version which
	//! can recursively compute the parent RelevantAtomSet.
	//! Additionally, this caches the set within the node for future reference.
	const RelevantAtomSet& compute_R(NodeT& node, BFWSStats& stats) {

		// If the R(s) has been previously computed and is cached, we return it straight away
		if (node._relevant_atoms != nullptr) return *node._relevant_atoms;


		// Otherwise, we compute it anew
		if (computation_of_R_necessary(node)) {

			// Throw a simulation from the node, and compute a set R[IW1] from there.
			bool verbose = !node.has_parent(); // Print info only on the s0 simulation
			auto evaluator = _sim_novelty_factory.create_compound_evaluator(_config.simulation_width);
			// TODO Fix this horrible hack
			if (_config.simulation_width==2) { stats.sim_table_created(1); stats.sim_table_created(2); }
			else  { assert(_config.simulation_width); stats.sim_table_created(1); }


			SimulationT simulator(_model, _featureset, evaluator, _simconfig, stats, verbose);

			node._helper = new AtomsetHelper(_problem.get_tuple_index(), simulator.compute_R(node.state));
			node._relevant_atoms = new RelevantAtomSet(*node._helper);

			//! MRJ: over states
			// node._relevant_atoms->init(node.state);
			//! Over feature sets
			node._relevant_atoms->init(_featureset.evaluate(node.state));

			if (!node.has_parent()) { // Log some info, but only for the seed state
				LPT_DEBUG("cout", "R(s_0)  (#=" << node._relevant_atoms->getHelper()._num_relevant << "): " << std::endl << *(node._relevant_atoms));
			}
		}


		else {
			// Copy the set R from the parent and update the set of relevant nodes with those that have been reached.
			node._relevant_atoms = new RelevantAtomSet(compute_R(*node.parent)); // This might trigger a recursive computation

			if (node.decreases_unachieved_subgoals()) {
				//! MRJ:
				//! Over states
				//node._relevant_atoms->init(node.state); // THIS IS ABSOLUTELY KEY E.G. IN BARMAN
				//! MRJ:  Over feature sets
				node._relevant_atoms->init(_featureset.evaluate(node.state));
			} else {
				//! MRJ: Over states
				//! node._relevant_atoms->update(node.state, nullptr);
				//! Old, deprecated use
				// node._relevant_atoms->update(node.state, &(node.parent->state));
				//! MRJ: Over feature sets
				node._relevant_atoms->update(_featureset.evaluate(node.state));
			}
		}

		return *node._relevant_atoms;
	}
	
	inline bool computation_of_R_necessary(const NodeT& node) const {
		if (_config.r_computation == SBFWSConfig::RComputation::Seed) return (!node.has_parent());
		else return !node.has_parent() || node.decreases_unachieved_subgoals();
	}
	
protected:
	const Problem& _problem;
	
	const SBFWSConfig& _config;
	
	const IWRunConfig _simconfig;
	
	const NoveltyFactory<FeatureValueT> _sim_novelty_factory;
	
	const FeatureSetT& _featureset;
};





//! 
class RelevantAtomsCounterFactory {
public:
	template <typename NodeT, typename SimulationT>
	static RelevantAtomsCounterI<NodeT>* build(const SBFWSConfig& config, const Problem& problem);
};


} } // namespaces
