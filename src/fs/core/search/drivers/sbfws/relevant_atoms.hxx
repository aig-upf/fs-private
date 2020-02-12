
#pragma once

#include <fs/core/search/drivers/sbfws/iw_run.hxx>


namespace fs0 { class Problem; class L0Heuristic; }
namespace fs0::bfws { struct SBFWSConfig; class RelevantAtomSet; class BFWSStats; }


namespace fs0::bfws {


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
class L0RelevantAtomsCounter : public RelevantAtomsCounterI<NodeT> {
public:
    explicit L0RelevantAtomsCounter(const Problem& problem);
    ~L0RelevantAtomsCounter();
    unsigned count(NodeT& node, BFWSStats& stats) const override;

protected:
    L0Heuristic* _l0_heuristic;
};


//!
template <typename ModelT, typename NodeT, typename NoveltyEvaluatorT, typename FeatureSetT>
class SimulationBasedRelevantAtomsCounter : public RelevantAtomsCounterI<NodeT> {
public:
    using FeatureValueT = typename NoveltyEvaluatorT::FeatureValueT;

    SimulationBasedRelevantAtomsCounter(const ModelT& model, const SBFWSConfig& config, const FeatureSetT& features)  :
            _model(model),
            _problem(model.getTask()),
            _config(config),
            iwconfig_(config.complete_simulation, config.simulation_width, config._global_config),
            _sim_novelty_factory(_problem, config.evaluator_t, features.uses_extra_features(), config.simulation_width),
            _featureset(features)
    {}
    ~SimulationBasedRelevantAtomsCounter() = default;


    unsigned count(NodeT& node, BFWSStats& stats) const override {
        return compute_R(node, stats).num_reached();
    }

    std::vector<bool> throw_simulation(const State& state, BFWSStats& stats, bool verbose) const {
        // Throw a simulation from the node, and compute a set R of relevant atoms from there.
        auto evaluator = _sim_novelty_factory.create_compound_evaluator(_config.simulation_width);
        if (_config.simulation_width==2) { stats.sim_table_created(1); stats.sim_table_created(2); }
        else  { assert(_config.simulation_width); stats.sim_table_created(1); }


        using IWNodeT = IWRunNode<State, typename ModelT::ActionType>;
        using IWRunT = IWRun<IWNodeT, ModelT, NoveltyEvaluatorT, FeatureSetT>;
        IWRunT simulator(_model, _featureset, evaluator, iwconfig_, stats, verbose);
        return simulator.compute_R(state);
    }


    //! Compute the RelevantAtomSet that corresponds to the given node, and from which
    //! the counter #r(node) can be obtained. This implements a lazy version which
    //! can recursively compute the parent RelevantAtomSet.
    //! Additionally, this caches the set within the node for future reference.
    const RelevantAtomSet& compute_R(NodeT& node, BFWSStats& stats) const {

        // If the R(s) has been previously computed and is cached, we return it straight away
        if (node._relevant_atoms != nullptr) return *node._relevant_atoms;


        // Otherwise, we compute it anew
        if (computation_of_R_necessary(node)) {
            bool verbose = !node.has_parent(); // Print info only on the s0 simulation
            auto R = throw_simulation(node.state, stats, verbose);
            node._helper = new AtomsetHelper(_problem.get_tuple_index(), R);
            node._relevant_atoms = new RelevantAtomSet(*node._helper);

            //! MRJ: over states
             node._relevant_atoms->init(node.state);
            //! Over feature sets
//			node._relevant_atoms->init(_featureset.evaluate(node.state));

            if (!node.has_parent()) { // Log some info, but only for the seed state
                LPT_DEBUG("cout", "R(s_0)  (#=" << node._relevant_atoms->getHelper()._num_relevant << "): " << std::endl << *(node._relevant_atoms));
            }
        }


        else {
            // Copy the set R from the parent and update the set of relevant nodes with those that have been reached.
            node._relevant_atoms = new RelevantAtomSet(compute_R(*node.parent, stats)); // This might trigger a recursive computation

            if (node.decreases_unachieved_subgoals()) {
                //! MRJ:
                //! Over states
                node._relevant_atoms->init(node.state); // THIS IS ABSOLUTELY KEY E.G. IN BARMAN
                //! MRJ:  Over feature sets
//				node._relevant_atoms->init(_featureset.evaluate(node.state));
            } else {
                //! MRJ: Over states
                //! node._relevant_atoms->update(node.state, nullptr);
                //! Old, deprecated use
                 node._relevant_atoms->update(node.state, &(node.parent->state));
                //! MRJ: Over feature sets
//				node._relevant_atoms->update(_featureset.evaluate(node.state));
            }
        }

        return *node._relevant_atoms;
    }

    inline bool computation_of_R_necessary(const NodeT& node) const {
        if (_config.r_computation == SBFWSConfig::RComputation::Seed) return (!node.has_parent());
        else return !node.has_parent() || node.decreases_unachieved_subgoals();
    }

protected:
    const ModelT& _model;

    const Problem& _problem;

    const SBFWSConfig& _config;

    const IWRunConfig iwconfig_;

    const NoveltyFactory<FeatureValueT> _sim_novelty_factory;

    const FeatureSetT& _featureset;
};



//! Choose how we're going to count #r values that track the number of relevant
//! achieved atoms depdengin on the command-line configuration
class RelevantAtomsCounterFactory {
public:
    template <typename StateModelT, typename NodeT, typename NoveltyEvaluatorT, typename FeatureSetT>
    static RelevantAtomsCounterI<NodeT>* build(const StateModelT& model, const SBFWSConfig& config, const FeatureSetT& features)
    {
        if (config.relevant_set_type == SBFWSConfig::RelevantSetType::None) {
            return new NullRelevantAtomsCounter<NodeT>();
        }

        if (config.relevant_set_type == SBFWSConfig::RelevantSetType::L0) {
            return new L0RelevantAtomsCounter<NodeT>(model.getTask());
        }

        return new SimulationBasedRelevantAtomsCounter<StateModelT, NodeT, NoveltyEvaluatorT, FeatureSetT>(model, config, features);
    }
};


} // namespaces
