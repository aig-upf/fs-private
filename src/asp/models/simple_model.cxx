
#include <asp/models/simple_model.hxx>
#include <asp/models/helper.hxx>
#include <asp/lp_handler.hxx>
#include <problem.hxx>
#include <state.hxx>
#include <actions/ground_action.hxx>
#include <languages/fstrips/formulae.hxx>
#include <languages/fstrips/effects.hxx>
#include <utils/printers/helper.hxx>
#include <utils/printers/actions.hxx>

namespace fs0 { namespace asp {

using fs0::print::normalize;

SimpleModel::SimpleModel(const Problem& problem) :
	BaseModel(problem),
	 _goal_atoms(problem.getGoalConditions()->all_atoms()),
	 _action_index(ModelHelper::compute_action_index(problem))
{
	if (!dynamic_cast<fs::Conjunction::cptr>(problem.getGoalConditions())) {
		throw std::runtime_error("ASP heuristic available only for goals which are conjunctions of atoms");
	}
}

std::vector<std::string> SimpleModel::build_domain_rules(bool optimize) const {
	std::vector<std::string> rules;
	const ProblemInfo& info = _problem.getProblemInfo();
	
	// Types
	auto types = info.getTypeObjects();
	for (unsigned type_id = 0; type_id < types.size(); ++type_id) {
		std::string type_name = normalize(info.getTypename(type_id));
		for (unsigned object_id:types.at(type_id)) {
			std::string object_name = normalize(info.deduceObjectName(object_id, type_id));
			rules.push_back(type_name + "(" + object_name + ").");
		}
	}
	
	// The seed atoms
// 	rules.push_back("P :- seed(P).");

	// The goal integrity constraints:
	for (auto atom:_goal_atoms) {
		auto processed = process_atom(atom);
		if (!processed.second) throw std::runtime_error("Negated atoms not yet supported");
		rules.push_back(":- not " +  processed.first + ".");
	}
	
	// Action rules
	for (auto action:_problem.getGroundActions()) {
		process_ground_action(*action, rules);
	}
	
	// Standard directives
	if (optimize) {
		rules.push_back("#minimize {1, A : asupported(A)}.");
	}
	rules.push_back("#show.");
	rules.push_back("#show A : asupported(A).");
	
	// Register additional, domain-dependent rules
	if (_problem.getLPHandler()) {
		_problem.getLPHandler()->on_domain_rules(_problem, rules);
	}
	return rules;
}

void SimpleModel::process_ground_action(const GroundAction& action, std::vector<std::string>& rules) const {
	auto precondition = action.getPrecondition();
	std::string action_name = normalize(fs0::print::action_name(action));
	
	if (!dynamic_cast<fs::Conjunction::cptr>(precondition)) { // TODO - Perform this only once
		throw std::runtime_error("ASP heuristic available only for goals which are conjunctions of atoms");
	}
	
	// For each (add) effect 'p' of the given action 'a', we build a rule of the form 'p :- pre(a), a.'
	
	std::string prec_body;
	auto atoms = precondition->all_atoms();
	for (unsigned i = 0; i < atoms.size(); ++i) { // TODO - This, in general, should be performed only once
		auto processed = process_atom(atoms[i]);
		if (!processed.second) throw std::runtime_error("Negated atoms not yet supported");
		prec_body += processed.first + ", ";
	}
	
	rules.push_back("{ asupported(" + action_name  + ") }.");
	prec_body += "asupported(" + action_name  + ").";
	
	// action (add-) effect rules
	for (auto effect:action.getEffects()) {
		auto processed = process_effect(effect);
		if (processed.second) { // we have a positive ADD effect
			std::string rule = processed.first + " :- " + prec_body;
			rules.push_back(rule);
		}
	}
}

std::vector<std::string> SimpleModel::build_state_rules(const State& state) const {
	std::vector<std::string> atoms;
	const ProblemInfo& info = _problem.getProblemInfo();
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


std::pair<std::string, bool> SimpleModel::process_atom(const fs::AtomicFormula* atom) const {
	auto eq_atom = dynamic_cast<fs::EQAtomicFormula::cptr>(atom);
	if (!eq_atom) throw std::runtime_error("ASP heuristic available only for simple atoms");
	auto lhs = dynamic_cast<fs::StateVariable::cptr>(eq_atom->lhs());
	auto rhs = dynamic_cast<fs::IntConstant::cptr>(eq_atom->rhs());
	if (!lhs || !rhs) throw std::runtime_error("ASP heuristic available only for simple atoms and effects");
	
	if (rhs->getValue() != 1) throw std::runtime_error("ASP heuristic available only for simple preconditions");
	
	return std::make_pair(normalize(*lhs), rhs->getValue() == 1);
}

std::pair<std::string, bool> SimpleModel::process_effect(const fs::ActionEffect* effect) const {
	auto lhs = dynamic_cast<fs::StateVariable::cptr>(effect->lhs());
	auto rhs = dynamic_cast<fs::IntConstant::cptr>(effect->rhs());
	if (!lhs || !rhs) throw std::runtime_error("ASP heuristic available only for simple atoms and effects");
	return std::make_pair(normalize(*lhs), rhs->getValue() == 1);
}

} } // namespaces