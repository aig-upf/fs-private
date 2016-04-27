
#include <asp/asp_hplus.hxx>
#include <state.hxx>
#include <problem.hxx>
#include <aptk2/tools/logging.hxx>
#include <utils/printers/asp.hxx>


namespace fs0 { namespace asp {


ASPHPlus::ASPHPlus(const Problem& problem) :
	_handler(problem, true) // Use Clingo's optimization mode
{}

long ASPHPlus::evaluate(const State& seed) {
	auto solution = _handler.process(seed);
	
	if (solution.first == Gringo::SolveResult::UNSAT) return -1;
	assert(solution.first == Gringo::SolveResult::SAT);
	
	// Because we're using Clingo's optimization mode, we know that the obtained relaxed plan is optimal.
	LPT_DEBUG("asp", "Optimal ASP solution / Relaxed plan:" << std::endl << fs0::print::asp_model(solution.second));
	return solution.second.size();
}


} } // namespaces