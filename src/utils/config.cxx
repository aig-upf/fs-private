
#include <utils/config.hxx>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>


namespace pt = boost::property_tree;

namespace fs0 {
	
Config* Config::_instance = nullptr;

void Config::init(const std::string& filename) {
	if (_instance) throw std::runtime_error("Global configuration object already initialized");
	_instance = new Config(filename);
}

//! Retrieve the singleton instance, which has been previously initialized
Config& Config::instance() {
	if (!_instance) throw std::runtime_error("The global configuration object needs to be explicitly initialized before using it");
	return *_instance;
}
	
Config::Config(const std::string& filename)
	: _filename(filename)
{
	pt::ptree tree;
	pt::json_parser::read_json(filename, tree);
	
	std::string parsed;
	
	// Parse the type of relaxed plan extraction: propositional or supported
	parsed = tree.get<std::string>("heuristics.plan_extraction");
	if (parsed != "propositional" && parsed != "supported") throw std::runtime_error("Invalid configuration option for key heuristics.plan_extraction: " + parsed);
	if (parsed == "propositional") _rpg_extraction = RPGExtractionType::Propositional;
	else _rpg_extraction = RPGExtractionType::Supported;
	
	// Parse the type of action manager: gecode or hybrid
	parsed = tree.get<std::string>("action_manager");
	if (parsed != "gecode" && parsed != "hybrid") throw std::runtime_error("Invalid configuration option for key action_manager: " + parsed);
	if (parsed == "gecode") _action_manager = ActionManagerType::Gecode;
	else _action_manager = ActionManagerType::Hybrid;
	
	// Parse the type of action manager: gecode, hybrid, basic
	parsed = tree.get<std::string>("goal_manager");
	if (parsed != "gecode" && parsed != "hybrid" && parsed != "basic") throw std::runtime_error("Invalid configuration option for key goal_manager: " + parsed);
	if (parsed == "gecode") _goal_manager = GoalManagerType::Gecode;
	else if (parsed == "hybrid") _goal_manager = GoalManagerType::Hybrid;
	else _goal_manager = GoalManagerType::Basic;
	
}


Config::~Config() {}

std::ostream& Config::print(std::ostream& os) const {
	os << "Action manager type: " << ((_action_manager == ActionManagerType::Gecode) ? "Gecode" : "Hybrid") << std::endl;
	os << "Goal manager type: " << (_goal_manager == GoalManagerType::Gecode ? "Gecode" : (_goal_manager == GoalManagerType::Hybrid ? "Hybrid" : "Basic")) << std::endl;
	os << "Plan extraction type: " << ((_rpg_extraction == RPGExtractionType::Propositional) ? "Propositional" : "Supported") << std::endl;
	return os;
}




} // namespaces
