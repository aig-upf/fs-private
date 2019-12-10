

#include <fs/core/search/drivers/sbfws/relevant_atoms.hxx>
#include <fs/core/search/drivers/sbfws/sbfws.hxx>
#ifdef FS_HYBRID
    #include <fs/hybrid/heuristics/l2_norm.hxx>
#endif //FS_HYBRID

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

#ifdef FS_HYBRID

template <typename NodeT>
L2NormRelevantAtomsCounter<NodeT>::L2NormRelevantAtomsCounter(const Problem& problem) :
	_l2_norm(new hybrid::L2Norm(problem))
{
}

template <typename NodeT>
L2NormRelevantAtomsCounter<NodeT>::~L2NormRelevantAtomsCounter() {
	delete _l2_norm;
}

template <typename NodeT>
    unsigned L2NormRelevantAtomsCounter<NodeT>::count(NodeT& node, BFWSStats& stats) const {
	unsigned v = _l2_norm->ball_geodesic_index(node.state);
	return v;
}

#else
template <typename NodeT>
L2NormRelevantAtomsCounter<NodeT>::~L2NormRelevantAtomsCounter() {}
#endif


// explicit template instantiation
template class L0RelevantAtomsCounter<fs0::bfws::SBFWSNode<fs0::State, fs0::GroundAction>>;
template class L0RelevantAtomsCounter<fs0::bfws::SBFWSNode<fs0::State, fs0::LiftedActionID>>;
template class L2NormRelevantAtomsCounter<fs0::bfws::SBFWSNode<fs0::State, fs0::GroundAction>>;
template class L2NormRelevantAtomsCounter<fs0::bfws::SBFWSNode<fs0::State, fs0::LiftedActionID>>;

} } // namespaces
