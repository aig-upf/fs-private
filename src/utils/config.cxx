
#include <utils/config.hxx>
#include <fs_types.hxx>
#include <boost/property_tree/json_parser.hpp>


namespace pt = boost::property_tree;

namespace fs0 {
	
std::unique_ptr<Config> Config::_instance = nullptr;

void Config::init(const std::string& root, const std::unordered_map<std::string, std::string>& user_options, const std::string& filename) {
	if (_instance) throw std::runtime_error("Global configuration object already initialized");
	_instance = std::unique_ptr<Config>(new Config(root, user_options, filename));
}

//! Retrieve the singleton instance, which has been previously initialized
Config& Config::instance() {
	if (!_instance) throw std::runtime_error("The global configuration object needs to be explicitly initialized before using it");
	return *_instance;
}

template <typename OptionType>
OptionType parseOption(const pt::ptree& tree, const std::unordered_map<std::string, std::string>& user_options, const std::string& key, std::map<std::string, OptionType> allowed) {
	
	std::string parsed;
	
	auto it = user_options.find(key);
	if (it != user_options.end()) { // The user specified an option value, which thus has priority
		parsed = it->second;
	} else {
		parsed = tree.get<std::string>(key);
	}
	
	auto it2 = allowed.find(parsed);
	if (it2 == allowed.end()) {
		throw std::runtime_error("Invalid configuration option for key " + key + ": " + parsed);
	}
	
	return it2->second;
}

Config::Config(const std::string& root, const std::unordered_map<std::string, std::string>& user_options, const std::string& filename)
	: _user_options(user_options)
{
	load(filename); // Load the default options
}

void Config::load(const std::string& filename) {
	pt::json_parser::read_json(filename, _root);
	
	// Parse the type of relaxed plan extraction: propositional or extended
	_rpg_extraction = parseOption<RPGExtractionType>(_root, _user_options, "plan_extraction", {{"propositional", RPGExtractionType::Propositional}, {"extended", RPGExtractionType::Supported}});
	
	_goal_resolution = parseOption<CSPResolutionType>(_root, _user_options, "goal_resolution", {{"full", CSPResolutionType::Full}, {"approximate", CSPResolutionType::Approximate}});
	
	_precondition_resolution = parseOption<CSPResolutionType>(_root, _user_options, "precondition_resolution", {{"full", CSPResolutionType::Full}, {"approximate", CSPResolutionType::Approximate}});
	
	_goal_value_selection = parseOption<ValueSelection>(_root, _user_options, "goal_value_selection", {{"min_hmax", ValueSelection::MinHMax}, {"min_val", ValueSelection::MinVal}});
	
	_action_value_selection = parseOption<ValueSelection>(_root, _user_options, "action_value_selection", {{"min_hmax", ValueSelection::MinHMax}, {"min_val", ValueSelection::MinVal}});
	
	_support_priority = parseOption<SupportPriority>(_root, _user_options, "support_priority", {{"min_hmaxsum", SupportPriority::MinHMaxSum}, {"first", SupportPriority::First}});
	
	_novelty = parseOption<bool>(_root, _user_options, "novelty", {{"true", true}, {"false", false}});
	
	_delayed = parseOption<bool>(_root, _user_options, "delayed_evaluation", {{"true", true}, {"false", false}});
	
	_heuristic = parseOption<std::string>(_root, _user_options, "heuristic", {{"hff", "hff"}, {"hmax", "hmax"}});
}


std::ostream& Config::print(std::ostream& os) const {
	os << "Action Resolution:\t" << ((_goal_resolution == CSPResolutionType::Approximate) ? "Approximate" : "Full") << std::endl;
	os << "Goal Resolution:\t" << ((_goal_resolution == CSPResolutionType::Approximate) ? "Approximate" : "Full") << std::endl;
	os << "Plan Extraction:\t" << ((_rpg_extraction == RPGExtractionType::Propositional) ? "Propositional" : "Extended") << std::endl;
	os << "Goal CSP Value Selection:\t" << ((_goal_value_selection == ValueSelection::MinHMax) ? "Value with minimum h_max value" : "Minimum value") << std::endl;
	os << "Action CSP Value Selection:\t" << ((_action_value_selection == ValueSelection::MinHMax) ? "Value with minimum h_max value" : "Minimum value") << std::endl;
	os << "Support Priority:\t" << ((_support_priority == SupportPriority::MinHMaxSum) ? "Support minimizing the sum of h_max values" : "First support found") << std::endl;
	return os;
}




} // namespaces
