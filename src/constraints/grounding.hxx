
#pragma once

#include <string>
#include <fs_types.hxx>

namespace fs0 { namespace language { namespace fstrips { class Term; class Formula; class Axiom;}}}
namespace fs = fs0::language::fstrips;

namespace fs0 {

class ProblemInfo;

class Binding;

//! This exception is thrown whenever a variable cannot be resolved
class TooManyGroundAxiomsError : public std::runtime_error {
public:
	TooManyGroundAxiomsError( unsigned long num_axioms ) : std::runtime_error("The number of ground axioms is too high: " + std::to_string(num_axioms)) {}
};

class AxiomGrounder {
public:
	static const unsigned long MAX_GROUND_AXIOMS = 100000000;

	static std::vector<const fs::Axiom*> fully_ground(const std::vector<const fs::Axiom*>& axioms, const ProblemInfo& info);


protected:

};

} // namespaces
