
#include <asp/asp_rpg.hxx>
#include <problem.hxx>
#include <heuristics/relaxed_plan/direct_crpg.hxx>
#include <aptk2/tools/logging.hxx>
#include <utils/printers/asp.hxx>


namespace fs0 { namespace asp {


template <typename RPGBaseHeuristic>
ASPRPG<RPGBaseHeuristic>::ASPRPG(const Problem& problem, RPGBaseHeuristic&& heuristic) :
	_handler(problem, false), // There is no sense in using Clingo's optimization mode if we'll re-compute the RPG afterwards
	_heuristic(std::move(heuristic))
{}

template <typename RPGBaseHeuristic>
long ASPRPG<RPGBaseHeuristic>::evaluate(const State& seed) {
	auto solution = _handler.process(seed);
	
	// We want to obtain the set of ground actions that are part of the relaxed plan 
	// and then build the RPG taking those ground actions into account only.
	LPT_DEBUG("asp", "ASP solution:" << std::endl << fs0::print::asp_model(solution.second));
	auto whitelist = _handler.get_action_set(solution.second);
	return _heuristic.evaluate(seed, whitelist);
}


// explicit instantiations
template class ASPRPG<DirectCRPG>;
template class ASPRPG<DirectCHMax>;


} } // namespaces