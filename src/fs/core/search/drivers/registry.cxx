
#include <fs/core/problem.hxx>
#include <fs/core/search/drivers/registry.hxx>
#include <fs/core/search/drivers/iterated_width.hxx>
#include <fs/core/search/drivers/breadth_first_search.hxx>
#include <fs/core/search/drivers/sbfws/sbfws.hxx>
#include <fs/core/heuristics/unsat_goal_atoms.hxx>
#include <fs/core/actions/grounding.hxx>


namespace fs0 { namespace drivers {

EngineRegistry& EngineRegistry::instance() {
	static EngineRegistry theInstance;
	return theInstance;
}

EngineRegistry::EngineRegistry() {
	// We register the pre-configured search drivers on the instantiation of the singleton
	add("iw",  new IteratedWidthDriver<GroundStateModel>());
	add("sbfws",  new bfws::SBFWSDriver<SimpleStateModel>());
	add("bfs",  new BreadthFirstSearchDriver<GroundStateModel>());
}

EngineRegistry::~EngineRegistry() {
	for (const auto& elem:_creators) delete elem.second;
}

void EngineRegistry::add(const std::string& engine_name, Driver* creator) {
auto res = _creators.insert(std::make_pair(engine_name, creator));
	if (!res.second) throw std::runtime_error("Duplicate registration of engine creator for symbol " + engine_name);
}


Driver* EngineRegistry::get(const std::string& engine_name) {
	auto it = _creators.find(engine_name);
	if (it == _creators.end()) throw std::runtime_error("No engine creator has been registered for given engine name '" + engine_name + "'");
	return it->second;
}

} } // namespaces
