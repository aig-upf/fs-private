

#include <fs/core/search/drivers/sbfws/relevant_atoms.hxx>
#include <fs/core/search/drivers/sbfws/sbfws.hxx>

namespace fs0 { namespace bfws {

/****** L0-BASED #R COUNTER *****/
template <typename NodeT>
L0RelevantAtomsCounter<NodeT>::L0RelevantAtomsCounter(const Problem &problem)
    : _l0_heuristic(new L0Heuristic(problem))
{}

template <typename NodeT>
L0RelevantAtomsCounter<NodeT>::~L0RelevantAtomsCounter() { delete _l0_heuristic; }

template <typename NodeT>
unsigned L0RelevantAtomsCounter<NodeT>::count(NodeT& node, BFWSStats& stats) const {
	return (unsigned) _l0_heuristic->evaluate(node.state);
}



// explicit template instantiation
template class L0RelevantAtomsCounter<fs0::bfws::SBFWSNode<fs0::State, fs0::GroundAction>>;
template class L0RelevantAtomsCounter<fs0::bfws::SBFWSNode<fs0::State, fs0::LiftedActionID>>;

} } // namespaces
