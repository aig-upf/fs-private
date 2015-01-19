
#include <heuristics/null_heuristic.hxx>

namespace fs0 {

template <typename ProblemT>
void NullHeuristic<ProblemT>::eval( const State& s, float& h_val ) {
	h_val = 0.0f;
}

template <typename ProblemT>
void NullHeuristic<ProblemT>::eval( const State& s, float& h_val,  std::vector<aptk::Action_Idx>& pref_ops ) {
	h_val = 0.0f;
}

} // namespaces

