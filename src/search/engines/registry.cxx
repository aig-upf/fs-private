
#include <problem.hxx>
#include <search/engines/registry.hxx>
#include <search/engines/gbfs_constrained.hxx>
#include <search/engines/iterated_width.hxx>
#include <search/engines/breadth_first_search.hxx>
#include "gbfs_novelty.hxx"
#include <heuristics/relaxed_plan/constrained_relaxed_plan_heuristic.hxx>
#include <heuristics/relaxed_plan/constrained_hmax.hxx>
#include <constraints/direct/direct_rpg_builder.hxx>
#include <constraints/gecode/gecode_rpg_builder.hxx>

using namespace fs0::gecode;

namespace fs0 { namespace engines {

EngineRegistry& EngineRegistry::instance() {
	static EngineRegistry theInstance;
	return theInstance;
}

EngineRegistry::EngineRegistry() {
	// We register the pre-configured search engines on the instantiation of the singleton
	add("gbfs_chff",  new GBFSConstrainedHeuristicsCreator<ConstrainedRelaxedPlanHeuristic<GecodeRPGBuilder>, ConstrainedRelaxedPlanHeuristic<DirectRPGBuilder>>());
	add("gbfs_chmax",  new GBFSConstrainedHeuristicsCreator<ConstrainedHMaxHeuristic<GecodeRPGBuilder>, ConstrainedHMaxHeuristic<DirectRPGBuilder>>());
	add("iterated_width",  new IteratedWidthEngineCreator());
	add("gbfs_novelty",  new GBFSNoveltyEngineCreator());
	add("breadth_first_search",  new BreadthFirstSearchEngineCreator());
	
}

EngineRegistry::~EngineRegistry() {
	for (const auto elem:_creators) delete elem.second;
}

void EngineRegistry::add(const std::string& engine_name, EngineCreator::cptr creator) {
auto res = _creators.insert(std::make_pair(engine_name, creator));
	if (!res.second) throw new std::runtime_error("Duplicate registration of engine creator for symbol " + engine_name);
}


EngineCreator::cptr EngineRegistry::get(const std::string& engine_name) const {
	auto it = _creators.find(engine_name);
	if (it == _creators.end()) throw std::runtime_error("No engine creator has been registered for given engine name '" + engine_name + "'");
	return it->second;
}


} } // namespaces
