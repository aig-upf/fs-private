
#pragma once

#include <fs0_types.hxx>
#include <state_model.hxx>
#include <aptk2/search/interfaces/search_algorithm.hxx>
#include <search/fs0_node.hxx>

namespace fs0 { class Config; }

namespace fs0 { namespace engines {

typedef aptk::search::FS0_Node<fs0::State> FS0SearchNode;
typedef aptk::SearchAlgorithm<FS0StateModel> FS0SearchAlgorithm;
	
class EngineFactory {
public:
	static std::unique_ptr<FS0SearchAlgorithm> create(const Config& config, const FS0StateModel& model);
};


} } // namespaces
