
#pragma once

#include <iostream>
#include <sstream>
#include <vector>

#include <core_problem.hxx>

namespace aptk { namespace core { namespace solver {
	
//! Generates a core-planning whole problem.
void generate(const std::string& data_dir, aptk::core::Problem& prob);
	
} } } // namespaces
