
#include <search/drivers/bfws.hxx>
#include <search/events.hxx>
#include <search/utils.hxx>
#include <actions/ground_action_iterator.hxx>
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
  
GroundStateModel
BFWSDriver::setup(Problem& problem) const {
	problem.setGroundActions(ActionGrounder::fully_ground(problem.getActionData(), ProblemInfo::getInstance()));
	return GroundStateModel(problem); // By default we ground all actions and return a model with the problem as it is
}

BFWSDriver::Engine
BFWSDriver::create_f0(const Config& config, BFWSConfig& bfws_config, const NoveltyFeaturesConfiguration& feature_configuration, const GroundStateModel& model) {
	
	using EvaluatorT = EvaluationObserver<NodeT, NoveltyHeuristic>;
	using StatsT = StatsObserver<NodeT>;

	_heuristic = std::unique_ptr<NoveltyHeuristic>(new NoveltyHeuristic(model, bfws_config._max_width, feature_configuration));
	
	_handlers.push_back(std::unique_ptr<StatsT>(new StatsT(_stats)));
	_handlers.push_back(std::unique_ptr<EvaluatorT>(new EvaluatorT(*_heuristic, config.getNodeEvaluationType())));
	
	auto engine = new lapkt::StlBestFirstSearch<NodeT, NoveltyHeuristic, GroundStateModel>(model, *_heuristic);
	lapkt::events::subscribe(*engine, _handlers);

	return Engine(engine);
}



void 
BFWSDriver::search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	
	BFWSConfig bfws_config(config);
	GroundStateModel model = setup(problem);
	
	NoveltyFeaturesConfiguration feature_configuration(config);
	
	LPT_INFO("cout", "BFWS Configuration:");
	LPT_INFO("cout", "\tBFWS Type: " << bfws_config._type);
	LPT_INFO("cout", "\tMax novelty: " << bfws_config._max_width);
	LPT_INFO("cout", "\tFeature extraction: " << feature_configuration);
	
	
	if (bfws_config._type == BFWSConfig::Type::F0) {
		auto engine = create_f0(config, bfws_config, feature_configuration, model);
		Utils::do_search(*engine, model, out_dir, start_time, _stats);
	}
	
	if (bfws_config._type == BFWSConfig::Type::F1) {
// 		auto engine = create_f1(feature_configuration, model);
// 		SearchUtils::do_search(*this, model, out_dir, start_time, _stats);
	}
	
	throw std::runtime_error("Invalid BFWS type");
}


} } // namespaces
