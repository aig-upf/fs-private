

#include <fs/core/search/drivers/sbfws/relevant_atoms.hxx>
#include <fs/core/search/drivers/sbfws/sbfws.hxx>


namespace fs0 { namespace bfws {

//template <typename StateModelT, typename NodeT, typename SimulationT, typename NoveltyEvaluatorT, typename FeatureSetT>
//RelevantAtomsCounterI<NodeT>* RelevantAtomsCounterFactory::build(const StateModelT& model, const SBFWSConfig& config) {
//
//	if (config.relevant_set_type == SBFWSConfig::RelevantSetType::None) {
//		return new NullRelevantAtomsCounter<NodeT>(model.getTask());
//	}
//
//	if (config.relevant_set_type == SBFWSConfig::RelevantSetType::L0) {
//		return new L0RelevantAtomsCounter<NodeT>(model.getTask());
//	}
//
//	if (config.relevant_set_type == SBFWSConfig::RelevantSetType::G0) {
//		return new L2NormRelevantAtomsCounter<NodeT>(model.getTask());
//	}
//
//
//	return new SimulationBasedRelevantAtomsCounter<StateModelT, NodeT, SimulationT, NoveltyEvaluatorT, FeatureSetT>(model);
//}


/****** L0-BASED #R COUNTER *****/
template <typename NodeT>
L0RelevantAtomsCounter<NodeT>::L0RelevantAtomsCounter(const Problem &problem)
    : _l0_heuristic(new L0Heuristic(problem))
{}

template <typename NodeT>
L0RelevantAtomsCounter<NodeT>::~L0RelevantAtomsCounter() { delete _l0_heuristic; }

template <typename NodeT>
unsigned L0RelevantAtomsCounter<NodeT>::count(NodeT& node, BFWSStats& stats) const {
	auto v =  (unsigned) _l0_heuristic->evaluate(node.state);
	node._hash_r = v;
	return v;
}

/****** L2-BASED #R COUNTER *****/
template <typename NodeT>
L2NormRelevantAtomsCounter<NodeT>::~L2NormRelevantAtomsCounter() {}



///****** IW-SIMULATION-BASED #R COUNTER *****/
//template <typename ModelT, typename NodeT, typename SimulationT, typename NoveltyEvaluatorT, typename FeatureSetT>
//SimulationBasedRelevantAtomsCounter<ModelT, NodeT, SimulationT, NoveltyEvaluatorT, FeatureSetT>::
//SimulationBasedRelevantAtomsCounter(const ModelT& model, const SBFWSConfig& config, const FeatureSetT& features) :
//    _model(model),
//	_problem(model.getTask()),
//	_config(config),
//	_simconfig(config.complete_simulation, config.mark_negative_propositions, config.simulation_width, config._global_config),
//    _sim_novelty_factory(_problem, config.evaluator_t, features.uses_extra_features(), config.simulation_width),
//    _featureset(features)
//{
//}
//
//template <typename ModelT, typename NodeT, typename SimulationT, typename NoveltyEvaluatorT, typename FeatureSetT>
//SimulationBasedRelevantAtomsCounter<ModelT, NodeT, SimulationT, NoveltyEvaluatorT, FeatureSetT>::
//~SimulationBasedRelevantAtomsCounter() {}
//




#ifdef FS_HYBRID

#include <fs/hybrid/heuristics/l2_norm.hxx>

template <typename NodeT>
L2NormRelevantAtomsCounter<NodeT>::L2NormRelevantAtomsCounter(const Problem& problem) :
    _l2_norm(hybrid::L2Norm(problem))
{
}

template <typename NodeT>
L2NormRelevantAtomsCounter<NodeT>::~L2NormRelevantAtomsCounter() {
	delete _l2_norm;
}

template <typename NodeT>
L2NormRelevantAtomsCounter<NodeT>::L2NormRelevantAtomsCounter(const Problem& problem) :
	_l2_norm(hybrid::L2Norm(problem))
{
}

template <typename NodeT>
L2NormRelevantAtomsCounter<NodeT>::~L2NormRelevantAtomsCounter() {
	delete _l2_norm;
}

template <typename NodeT>
    unsigned L2NormRelevantAtomsCounter<NodeT>::count(NodeT& node, BFWSStats& stats) const {
	unsigned v = _l2_norm->ball_geodesic_index(node.state);
	node._hash_r = v;
	return v;
}



#endif


// explicit template instantiation
template class L0RelevantAtomsCounter<fs0::bfws::SBFWSNode<fs0::State, fs0::GroundAction>>;
template class L0RelevantAtomsCounter<fs0::bfws::SBFWSNode<fs0::State, fs0::LiftedActionID>>;
template class L2NormRelevantAtomsCounter<fs0::bfws::SBFWSNode<fs0::State, fs0::GroundAction>>;
template class L2NormRelevantAtomsCounter<fs0::bfws::SBFWSNode<fs0::State, fs0::LiftedActionID>>;

} } // namespaces
