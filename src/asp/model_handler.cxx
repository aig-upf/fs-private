
#include <asp/model_handler.hxx>
#include <asp/model.hxx>
#include <state.hxx>
#include <problem.hxx>
#include <heuristics/relaxed_plan/direct_crpg.hxx>
#include <heuristics/relaxed_plan/direct_chmax.hxx>
#include <utils/logging.hxx>
#include <utils/printers/asp.hxx>
#include <utils/printers/vector.hxx>


namespace fs0 { namespace asp {


ModelHandler::ModelHandler(const Problem& problem, bool optimize) :
	_clingo(std::make_shared<Clingo>()),
	_model(std::make_shared<Model>(problem)),
	_domain_rules(_model->build_domain_rules(optimize))
{
	FDEBUG("asp", "Setting up ASP-based RPG builder. See log files 'asp-domain.log' and 'asp-init.log' for a complete ASP model.");
	FDEBUG("asp-domain", "ASP model for the planning domain:" << std::endl << fs0::print::container(_domain_rules, "\n"));
	FDEBUG("asp-init", "ASP model for the initial state:" << std::endl << fs0::print::container(_model->build_state_rules(problem.getInitialState()), "\n"));
}

std::pair<Gringo::SolveResult, Clingo::Solution> ModelHandler::process(const State& seed) {
	FDEBUG("asp", "Solving ASP model for state " << seed);
	auto res = _clingo->solve(_domain_rules, _model->build_state_rules(seed));
	if (res.first == Gringo::SolveResult::UNKNOWN) throw std::runtime_error("Clingo returned UNKNOWN solve status");
	return res;
}

std::vector<unsigned> ModelHandler::get_action_set(const Clingo::Solution& model) const {
	return _model->get_action_set(model);
}


} } // namespaces