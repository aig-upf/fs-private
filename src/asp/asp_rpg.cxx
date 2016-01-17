
#include <asp/asp_rpg.hxx>
#include <asp/model.hxx>
#include <state.hxx>
#include <problem.hxx>
#include <heuristics/relaxed_plan/direct_crpg.hxx>
#include <heuristics/relaxed_plan/direct_chmax.hxx>
#include <utils/logging.hxx>
#include <utils/printers/asp.hxx>


namespace fs0 { namespace asp {



template <typename RPGBaseHeuristic>
ASPRPG<RPGBaseHeuristic>::ASPRPG(const Problem& problem, RPGBaseHeuristic&& heuristic, bool optimize) :
	_clingo(std::make_shared<Clingo>()),
	_model(std::make_shared<Model>(problem, optimize)),
	_optimize(optimize),
	_heuristic(std::move(heuristic))
{
	_model->build_base();
}

template <typename RPGBaseHeuristic>
long ASPRPG<RPGBaseHeuristic>::evaluate(const State& seed) {
	FDEBUG("asp", "Computing ASP model for state " << seed);
	auto res = _clingo->solve(_model->get_base_rules(), _model->build_state_rules(seed));
	
	if (res.first == Gringo::SolveResult::UNKNOWN) throw std::runtime_error("Clingo returned UNKNOWN solve status");
	if (res.first == Gringo::SolveResult::UNSAT) return -1;
	assert(res.first == Gringo::SolveResult::SAT);
	
	// If using optimization, we know that the obtained relaxed plan is optimal and therefore don't need to do anything else
	if (_optimize) {
		FDEBUG("asp", "Optimal model:" << std::endl << fs0::print::asp_model(res.second));
		return res.second.size();
	}
	
	// Otherwise, we want to obtain the set of ground actions that are part of the relaxed plan 
	// and then build the RPG taking those ground actions into account only.
	FDEBUG("asp", "ASP model:" << std::endl << fs0::print::asp_model(res.second));
	auto whitelist = _model->get_action_set(res.second);
	return _heuristic.evaluate(seed, whitelist);
}



// explicit instantiations
template class ASPRPG<DirectCRPG>;
template class ASPRPG<DirectCHMax>;


} } // namespaces