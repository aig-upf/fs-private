
#include <search/engines/registry.hxx>
#include <search/engines/gbfs_crpg.hxx>
#include <search/engines/iterated_width.hxx>
#include <search/engines/breadth_first_search.hxx>
#include <problem.hxx>


namespace fs0 { namespace engines {

EngineRegistry& EngineRegistry::instance() {
	static EngineRegistry theInstance;
	return theInstance;
}

EngineRegistry::EngineRegistry() {
	// We register the pre-configured search engines on the instantiation of the singleton
	add("gbfs_crpg",  new GBFSEngineCreator());
	add("iterated_width",  new IteratedWidthEngineCreator());
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
