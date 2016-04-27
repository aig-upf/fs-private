
#include <asp/model_handler.hxx>
#include <asp/models/base_model.hxx>
#include <asp/models/straight_model.hxx>
#include <asp/models/helper.hxx>
#include <state.hxx>
#include <problem.hxx>
#include <heuristics/relaxed_plan/direct_crpg.hxx>
#include <aptk2/tools/logging.hxx>
#include <utils/printers/asp.hxx>
#include <utils/printers/vector.hxx>


namespace fs0 { namespace asp {


ModelHandler::ModelHandler(const Problem& problem, bool optimize) :
	_clingo(std::make_shared<Clingo>()),
//	_model(std::make_shared<ChoiceRuleModel>(problem)),
//	_model(std::make_shared<SimpleModel>(problem)),
	_model(std::make_shared<StraightModel>(problem)),
	_domain_rules(_model->build_domain_rules(optimize))
{
	LPT_DEBUG("asp", "Setting up ASP-based RPG builder. See log files 'asp-domain.log' and 'asp-init.log' for a complete ASP model.");
	LPT_DEBUG("asp-domain", "ASP model for the planning domain:" << std::endl << fs0::print::container(_domain_rules, "\n"));
	LPT_DEBUG("asp-init", "ASP model for the initial state:" << std::endl << fs0::print::container(_model->build_state_rules(problem.getInitialState()), "\n"));
}

std::pair<Gringo::SolveResult, Clingo::Solution> ModelHandler::process(const State& seed) {
	LPT_DEBUG("asp", "Solving ASP model for state " << seed);
	auto res = _clingo->solve(_domain_rules, _model->build_state_rules(seed));
	if (res.first == Gringo::SolveResult::UNKNOWN) throw std::runtime_error("Clingo returned UNKNOWN solve status");
	return res;
}

std::vector<unsigned> ModelHandler::get_action_set(const Clingo::Solution& model) const {
	return ModelHelper::compute_action_set(model, _model->get_action_idx());
}


} } // namespaces
