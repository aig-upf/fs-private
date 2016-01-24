
#include <asp/clingo.hxx>

#include <asp/models/helper.hxx>
#include <problem.hxx>
#include <actions/ground_action.hxx>
#include <utils/printers/helper.hxx>
#include <utils/printers/actions.hxx>

namespace fs0 { namespace asp {

using fs0::print::normalize;

std::unordered_map<std::string, unsigned> ModelHelper::compute_action_index(const Problem& problem) {
	std::unordered_map<std::string, unsigned> index;
	for (const GroundAction* action:problem.getGroundActions()) {
		std::string action_name = normalize(fs0::print::action_name(*action));
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


} } // namespaces