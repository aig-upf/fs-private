
#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <asp/models/base_model.hxx>

namespace Gringo { struct Value; }

namespace fs0 { namespace asp {

class ModelHelper {
public:
	
	//! A quick helper to compute an index of ground action names to action IDs
	static std::unordered_map<std::string, unsigned> compute_action_index(const Problem& problem);
	
	//! Returns the set of ground action IDs which are part of the relaxed plan
	//! represented by a given ASP solution
	static std::vector<unsigned> compute_action_set(const std::vector<Gringo::Value>& model, const std::unordered_map<std::string, unsigned>& index);
};

} } // namespaces
