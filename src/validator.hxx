
#pragma once

namespace fs0 {

class ProblemInfo;
class Problem;
class State;

//! A class to perform some static validation checks
class Validator {
public:
	static bool validate_state(const State& state, const ProblemInfo& info);
	
	static bool validate_problem_info(const ProblemInfo& info);
	
	static void validate_problem(const Problem& problem, const ProblemInfo& info);
};

} // namespaces
