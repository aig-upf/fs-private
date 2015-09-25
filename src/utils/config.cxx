
#include <utils/config.hxx>
#include <fs0_types.hxx>
#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>


namespace pt = boost::property_tree;

namespace fs0 {
	
std::unique_ptr<Config> Config::_instance = nullptr;

void Config::init(const std::string& filename) {
	if (_instance) throw std::runtime_error("Global configuration object already initialized");
	_instance = std::unique_ptr<Config>(new Config(filename));
	validateConfig(*_instance);
}

//! Retrieve the singleton instance, which has been previously initialized
Config& Config::instance() {
	if (!_instance) throw std::runtime_error("The global configuration object needs to be explicitly initialized before using it");
	return *_instance;
}



template <typename OptionType>
OptionType parseOption(const pt::ptree& tree, const std::string& key, std::map<std::string, OptionType> allowed) {
	std::string parsed = tree.get<std::string>(key);
	
	auto it = allowed.find(parsed);
	if (it == allowed.end()) {
		throw std::runtime_error("Invalid configuration option for key " + key + ": " + parsed);
	}
	
	return it->second;
}

Config::Config(const std::string& filename)
	: _filename(filename)
{
	pt::json_parser::read_json(filename, _root);
	
	_engine_tag = getOption<std::string>("engine.tag");
	
	// Parse the type of relaxed plan extraction: propositional or supported
	_rpg_extraction = parseOption<RPGExtractionType>(_root, "heuristics.plan_extraction", {{"propositional", RPGExtractionType::Propositional}, {"supported", RPGExtractionType::Supported}});
	
	// Parse the type of action manager: gecode or hybrid
	_action_manager = parseOption<ActionManagerType>(_root, "action_manager", {{"hybrid", ActionManagerType::Hybrid}, {"gecode", ActionManagerType::Gecode}});
	
	// Parse the type of action manager: gecode, hybrid, direct
	_goal_manager = parseOption<GoalManagerType>(_root, "goal_manager", {{"gecode", GoalManagerType::Gecode}, {"hybrid", GoalManagerType::Hybrid}, {"direct", GoalManagerType::Direct}});
	
	_goal_resolution = parseOption<CSPResolutionType>(_root, "goal_resolution", {{"full", CSPResolutionType::Full}, {"approximate", CSPResolutionType::Approximate}});
	_precondition_resolution = parseOption<CSPResolutionType>(_root, "precondition_resolution", {{"full", CSPResolutionType::Full}, {"approximate", CSPResolutionType::Approximate}});

}


Config::~Config() {}

// Some basic checks for invalid combinations of configuration options
void Config::validateConfig(const Config& config) {
	if (config.getGoalManagerType() != GoalManagerType::Gecode && config.getGoalResolutionType() == CSPResolutionType::Full) {
		throw InvalidConfiguration("Full Goal CSP resolution can only be performed with a Gecode Goal Manager");
	}
	
	if (config.getActionPreconditionResolutionType() == CSPResolutionType::Approximate) {
		throw InvalidConfiguration("Action CSP approximate resolution not yet implemented");
	}	
}

std::ostream& Config::print(std::ostream& os) const {
	os << "Action Manager:\t\t" << ((_action_manager == ActionManagerType::Gecode) ? "Gecode" : "Hybrid") << std::endl;
	os << "Action Resolution:\t" << ((_goal_resolution == CSPResolutionType::Approximate) ? "Approximate" : "Full") << std::endl;
	os << "Goal Manager:\t\t" << (_goal_manager == GoalManagerType::Gecode ? "Gecode" : (_goal_manager == GoalManagerType::Hybrid ? "Hybrid" : "Direct")) << std::endl;
	os << "Goal Resolution:\t" << ((_goal_resolution == CSPResolutionType::Approximate) ? "Approximate" : "Full") << std::endl;
	os << "Plan Extraction:\t" << ((_rpg_extraction == RPGExtractionType::Propositional) ? "Propositional" : "Supported") << std::endl;
	return os;
}




} // namespaces
