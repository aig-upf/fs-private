
#include <search/drivers/bfws/bfws.hxx>
#include <search/events.hxx>
#include <search/utils.hxx>
#include <actions/ground_action_iterator.hxx>
#include <search/drivers/smart_effect_driver.hxx>
#include <search/drivers/setups.hxx>
#include <actions/grounding.hxx>
#include <problem_info.hxx>

namespace fs0 { namespace drivers {


BFWSConfig::BFWSConfig(const Config& config) :
	_type(parse_type(config.getOption<std::string>("bfws.tag"))),
	_max_width(config.getOption<int>("width.max_novelty"))
{}

BFWSConfig::Type
BFWSConfig::parse_type(const std::string& type) {
	if (type == "f0") return Type::F0;
	if (type == "f1") return Type::F1;
	if (type == "f2") return Type::F2;
	if (type == "f5") return Type::F5;
	throw std::runtime_error("Invalid BFWS type tag: " + type);
}


std::ostream& operator<<(std::ostream &o, BFWSConfig::Type type) {
	switch(type) {
		case BFWSConfig::Type::F0: return o << "F0";
		case BFWSConfig::Type::F1: return o << "F1";
		case BFWSConfig::Type::F2: return o << "F2";
		case BFWSConfig::Type::F5: return o << "F5";
		default: return o << "(invalid value)";
	}
}


} } // namespaces
