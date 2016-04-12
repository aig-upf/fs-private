
#include <asp/models/straight_model.hxx>
#include <asp/models/helper.hxx>
#include <asp/lp_handler.hxx>
#include <problem.hxx>
#include <state.hxx>
#include <actions/actions.hxx>
#include <utils/printers/helper.hxx>
#include <utils/printers/actions.hxx>
#include <languages/fstrips/formulae.hxx>

namespace fs0 { namespace asp {

using fs0::print::normalize;

StraightModel::StraightModel(const Problem& problem) :
	BaseModel(problem),
	 _action_index(ModelHelper::compute_action_index(problem))
{
	if (!dynamic_cast<fs::Conjunction::cptr>(problem.getGoalConditions())) {
		throw std::runtime_error("ASP heuristic available only for goals which are conjunctions of atoms");
	}
}

std::vector<std::string> StraightModel::build_domain_rules(bool optimize) const {
	std::vector<std::string> rules;
	
	// Type rules
	ModelHelper::add_type_rules(_problem, rules);
	
	// The seed atoms
// 	rules.push_back("P :- seed(P).");

	// The goal integrity constraints:
	auto goal_atoms = _problem.getGoalConditions()->all_atoms();
	for (auto atom:goal_atoms) {
		auto processed = ModelHelper::process_atom(atom);
		if (!processed.second) throw std::runtime_error("Negated atoms not yet supported");
		rules.push_back(":- not " +  processed.first + ".");
	}
	
	// Action rules
	for (auto action:_problem.getGroundActions()) {
		process_ground_action(*action, rules);
	}
	
	// Standard directives
	if (optimize) {
		rules.push_back("#minimize {1, A : _applicable(A)}.");
	}
	rules.push_back("#show.");
	rules.push_back("#show A : _applicable(A).");
	
	// Register additional, domain-dependent rules
	if (_problem.getLPHandler()) {
		_problem.getLPHandler()->on_domain_rules(_problem, rules);
	}
	return rules;
}

void StraightModel::process_ground_action(const GroundAction& action, std::vector<std::string>& rules) const {
	auto precondition = action.getPrecondition();
	std::string action_name = normalize(fs0::print::action_header(action));
	
	if (!dynamic_cast<fs::Conjunction::cptr>(precondition)) { // TODO - Perform this only once
		throw std::runtime_error("ASP heuristic available only for goals which are conjunctions of atoms");
	}
	
	// For each (add) effect 'p' of the given action 'a', we build a rule of the form 'p :- pre(a), a.'
	
	std::string prec_body;
	auto atoms = precondition->all_atoms();
	for (unsigned i = 0; i < atoms.size(); ++i) { // TODO - This, in general, should be performed only once
		auto processed = ModelHelper::process_atom(atoms[i]);
		if (!processed.second) throw std::runtime_error("Negated atoms not yet supported");
		prec_body += processed.first;
		if (i < atoms.size() - 1) prec_body += ", ";
	}
	
	// Action precondition rules
	rules.push_back("_applicable(" + action_name  + ") :- " + prec_body + ".");
	
	// action (add-) effect rules
	for (auto effect:action.getEffects()) {
		auto processed = ModelHelper::process_effect(effect);
		if (processed.second) { // we have a positive ADD effect
			std::string rule = processed.first + " :- _applicable(" + action_name + ").";
			rules.push_back(rule);
		}
	}
}

std::vector<std::string> StraightModel::build_state_rules(const State& state) const {
	std::vector<std::string> atoms;
	const ProblemInfo& info = _ProblemInfo::getInstance();
	const auto& values = state.getValues();
	for (unsigned i = 0; i < values.size(); ++i) {
		if (values.at(i) == 1) {
			const std::string& atom_name = info.getVariableName(i);
			atoms.push_back(normalize(atom_name) + ".");
			
		}
	}
	
	if (_problem.getLPHandler()) {
		_problem.getLPHandler()->on_state_rules(_problem, state, atoms);
	}
	return atoms;
}

} } // namespaces
