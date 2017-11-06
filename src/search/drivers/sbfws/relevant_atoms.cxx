
#include "relevant_atoms.hxx"
#include "config.hxx"

#include <heuristics/l0.hxx>
#include <modules/hybrid/hybrid.hxx>


namespace fs0 { namespace bfws {

template <typename NodeT, typename SimulationT>
RelevantAtomsCounterI<NodeT>* RelevantAtomsCounterFactory::build(const SBFWSConfig& config, const Problem& problem) {
	
	if (config.relevant_set_type == SBFWSConfig::RelevantSetType::None) {
		return new NullRelevantAtomsCounter<NodeT>(problem);
	}
	
	if (config.relevant_set_type == SBFWSConfig::RelevantSetType::L0) {
		return new L0Heuristic(problem);
	}
	
	if (config.relevant_set_type == SBFWSConfig::RelevantSetType::G0) {
		return new L2NormRelevantAtomsCounter(problem);
	}
	
	
	return new SimulationBasedRelevantAtomsCounter<NodeT, SimulationT>(problem);
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
unsigned L2NormRelevantAtomsCounter<NodeT>::count(NodeT& node) const {
	unsigned v = _l2_norm->ball_geodesic_index(node.state);
	node._hash_r = v;
	return v;
}


template <typename NodeT>
L0RelevantAtomsCounter<NodeT>::L2NormRelevantAtomsCounter(const Problem& problem) :
	_l0_heuristic(L0Heuristic(problem))
{
}

template <typename NodeT>
L0RelevantAtomsCounter<NodeT>::~L2NormRelevantAtomsCounter() {
	delete _l0_heuristic;
}

template <typename NodeT>
unsigned L0RelevantAtomsCounter<NodeT>::count(NodeT& node) const {
	unsigned v =  _l0_heuristic->evaluate(node.state);
	node._hash_r = v;
	return v;
}



template <typename NodeT>
SimulationBasedRelevantAtomsCounter<NodeT>::L2NormRelevantAtomsCounter(const SBFWSConfig& config, const Problem& problem) :
	_problem(problem),
	_config(config),
	_simconfig(config.complete_simulation, config.mark_negative_propositions, config.simulation_width, config._global_config)
{
}

template <typename NodeT>
SimulationBasedRelevantAtomsCounter<NodeT>::~L2NormRelevantAtomsCounter() {
}

template <typename NodeT>
unsigned SimulationBasedRelevantAtomsCounter<NodeT>::count(NodeT& node) const {
	
	
	
}






} } // namespaces
