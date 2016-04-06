
#include <asp/clingo.hxx>

#include <asp/models/helper.hxx>
#include <problem.hxx>
#include <actions/actions.hxx>
#include <utils/printers/helper.hxx>
#include <utils/printers/actions.hxx>
#include <languages/fstrips/formulae.hxx>
#include <languages/fstrips/effects.hxx>

namespace fs0 { namespace asp {

using fs0::print::normalize;

std::unordered_map<std::string, unsigned> ModelHelper::compute_action_index(const Problem& problem) {
	std::unordered_map<std::string, unsigned> index;
	for (const GroundAction* action:problem.getGroundActions()) {
		std::string action_name = normalize(fs0::print::action_header(*action));
		index.insert(std::make_pair(action_name, action->getId()));
	}
	return index;
}

std::vector<unsigned> ModelHelper::compute_action_set(const std::vector<Gringo::Value>& model, const std::unordered_map<std::string, unsigned>& index) {
	std::vector<unsigned> actions;
	for (auto &atom : model) {
		std::string action_name = fs0::print::to_string(atom);
		auto it = index.find(action_name);
		assert(it != index.end());
		actions.push_back(it->second);
	}
	return actions;
}

void ModelHelper::add_type_rules(const Problem& problem, std::vector<std::string>& rules) {
	const ProblemInfo& info = problem.getProblemInfo();
	auto types = info.getTypeObjects();
	for (unsigned type_id = 0; type_id < types.size(); ++type_id) {
		std::string type_name = normalize(info.getTypename(type_id));
		for (unsigned object_id:types.at(type_id)) {
			std::string object_name = normalize(info.deduceObjectName(object_id, type_id));
			rules.push_back(type_name + "(" + object_name + ").");
		}
	}
}

std::pair<std::string, bool> ModelHelper::process_atom(const fs::AtomicFormula* atom) {
	auto eq_atom = dynamic_cast<fs::EQAtomicFormula::cptr>(atom);
	if (!eq_atom) throw std::runtime_error("ASP heuristic available only for simple atoms");
	auto lhs = dynamic_cast<fs::StateVariable::cptr>(eq_atom->lhs());
	auto rhs = dynamic_cast<fs::IntConstant::cptr>(eq_atom->rhs());
	if (!lhs || !rhs) throw std::runtime_error("ASP heuristic available only for simple atoms and effects");
	
	if (rhs->getValue() != 1) throw std::runtime_error("ASP heuristic available only for simple preconditions");
	
	return std::make_pair(normalize(*lhs), rhs->getValue() == 1);
}

std::pair<std::string, bool> ModelHelper::process_effect(const fs::ActionEffect* effect) {
	auto lhs = dynamic_cast<fs::StateVariable::cptr>(effect->lhs());
	auto rhs = dynamic_cast<fs::IntConstant::cptr>(effect->rhs());
	if (!lhs || !rhs) throw std::runtime_error("ASP heuristic available only for simple atoms and effects");
	return std::make_pair(normalize(*lhs), rhs->getValue() == 1);
}


} } // namespaces