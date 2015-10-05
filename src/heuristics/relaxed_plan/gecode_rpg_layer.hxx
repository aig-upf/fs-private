
#pragma once

#include <gecode/int.hh>
#include <fs0_types.hxx>


namespace fs0 {

class RelaxedState;

class GecodeRPGLayer {
protected:
	//! The set of possible value for every state variable
	std::vector<Gecode::IntSet> _domains;

public:
	explicit GecodeRPGLayer(const std::vector<std::vector<int>>& domains);
	explicit GecodeRPGLayer(const RelaxedState& layer);
	
	const Gecode::IntSet& get_domain(VariableIdx variable) const { return _domains.at(variable); }

	//! The only way to construct a relaxed state is from a non-relaxed state.
	GecodeRPGLayer(const GecodeRPGLayer& state)  = delete;
// 	GecodeRPGLayer& operator=(const GecodeRPGLayer& rhs) = delete;
	bool operator==(const GecodeRPGLayer& rhs) = delete;
	
	//! Prints a representation of the state to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const GecodeRPGLayer&  state) { return state.print(os); }
	std::ostream& print(std::ostream& os) const;
};

} // namespaces

