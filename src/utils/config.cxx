
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
	
	// Parse the type of relaxed plan extraction: propositional or extended
	_rpg_extraction = parseOption<RPGExtractionType>(_root, "heuristics.plan_extraction", {{"propositional", RPGExtractionType::Propositional}, {"extended", RPGExtractionType::Supported}});
	
	// Parse the type of action manager: gecode, direct-if-possible, direct
	_csp_manager = parseOption<CSPManagerType>(_root, "csp.manager", {{"gecode", CSPManagerType::Gecode}, {"asp", CSPManagerType::ASP}, {"direct_if_possible", CSPManagerType::DirectIfPossible}, {"direct", CSPManagerType::Direct}});
	_csp_model = parseOption<CSPModel>(_root, "csp.model", {
									{"action", CSPModel::GroundedActionCSP},
									{"effect", CSPModel::GroundedEffectCSP},
									{"action_schema", CSPModel::ActionSchemaCSP},
									{"effect_schema", CSPModel::EffectSchemaCSP},
				});
	
	_goal_resolution = parseOption<CSPResolutionType>(_root, "csp.goal_resolution", {{"full", CSPResolutionType::Full}, {"approximate", CSPResolutionType::Approximate}});
	_precondition_resolution = parseOption<CSPResolutionType>(_root, "csp.precondition_resolution", {{"full", CSPResolutionType::Full}, {"approximate", CSPResolutionType::Approximate}});
	
	_novelty_constraint = parseOption<bool>(_root, "csp.novelty_constraint", {{"yes", true}, {"no", false}});
	
	_element_dont_care_optimization = parseOption<bool>(_root, "csp.dont_care_optimization", {{"yes", true}, {"no", false}});
	
	_lifted_planning = parseOption<bool>(_root, "lifted_planning", {{"yes", true}, {"no", false}});
	
	_goal_value_selection = parseOption<ValueSelection>(_root, "csp.goal_value_selection", {{"min_hmax", ValueSelection::MinHMax}, {"min_val", ValueSelection::MinVal}});
	_action_value_selection = parseOption<ValueSelection>(_root, "csp.action_value_selection", {{"min_hmax", ValueSelection::MinHMax}, {"min_val", ValueSelection::MinVal}});
	
	_support_priority = parseOption<SupportPriority>(_root, "csp.support_priority", {{"min_hmaxsum", SupportPriority::MinHMaxSum}, {"first", SupportPriority::First}});
	
	_asp_optimization = parseOption<bool>(_root, "asp.optimize", {{"yes", true}, {"no", false}});
}


Config::~Config() {}

// Some basic checks for invalid combinations of configuration options
void Config::validateConfig(const Config& config) {
	if (config.getCSPManagerType() == CSPManagerType::Direct && config.getGoalResolutionType() == CSPResolutionType::Full) {
		throw InvalidConfiguration("Full Goal CSP resolution can't be performed with a Direct Goal Manager");
	}
	
	if (config.getActionPreconditionResolutionType() == CSPResolutionType::Approximate) {
		throw InvalidConfiguration("Action CSP approximate resolution not yet implemented");
	}	
}

std::ostream& Config::print(std::ostream& os) const {
	
	std::map<CSPModel, std::string> model_str =  {
	{CSPModel::GroundedActionCSP, "1 CSP per action"},
	{CSPModel::GroundedEffectCSP, "1 CSP per effect"},
	{CSPModel::ActionSchemaCSP, "1 CSP per action schema"},
	{CSPModel::EffectSchemaCSP, "1 CSP per effect schema"}
	};
	
	os << "Action Resolution:\t" << ((_goal_resolution == CSPResolutionType::Approximate) ? "Approximate" : "Full") << std::endl;
	os << "CSP Manager:\t\t" << (_csp_manager == CSPManagerType::Gecode ? "Gecode" : (_csp_manager == CSPManagerType::ASP ? "ASP" : (_csp_manager == CSPManagerType::DirectIfPossible ? "Direct-If-Possible" : "Direct"))) << std::endl;
	os << "CSP Model:\t" << model_str[_csp_model] << std::endl;
	os << "Goal Resolution:\t" << ((_goal_resolution == CSPResolutionType::Approximate) ? "Approximate" : "Full") << std::endl;
	os << "Plan Extraction:\t" << ((_rpg_extraction == RPGExtractionType::Propositional) ? "Propositional" : "Extended") << std::endl;
	
	os << "Goal CSP Value Selection:\t" << ((_goal_value_selection == ValueSelection::MinHMax) ? "Value with minimum h_max value" : "Minimum value") << std::endl;
	os << "Action CSP Value Selection:\t" << ((_action_value_selection == ValueSelection::MinHMax) ? "Value with minimum h_max value" : "Minimum value") << std::endl;
	os << "Support Priority:\t" << ((_support_priority == SupportPriority::MinHMaxSum) ? "Support minimizing the sum of h_max values" : "First support found") << std::endl;
	os << "Using Novelty Constraint?:\t" << ((_novelty_constraint) ? "Yes" : "No") << std::endl;
	os << "Using Element Don't Care Optimization?:\t" << ((_element_dont_care_optimization) ? "Yes" : "No") << std::endl;
	return os;
}




} // namespaces
