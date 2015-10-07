
#pragma once

#include <gecode/int.hh>
#include <fs0_types.hxx>


namespace fs0 { class ProblemInfo; class State; class Atom; }

namespace fs0 { namespace gecode {

class GecodeRPGLayer {
protected:
	//! The set of possible value for every state variable
	std::vector<Gecode::IntSet> _domains;
	
	//! A vector indexing each atom through its variable
	std::vector<Domain> _index;

	//! The set of variable deltas, i.e. only those values that were newly achieved in the previous RPG layer
	std::vector<Gecode::IntSet> _deltas;
	
public:
	explicit GecodeRPGLayer(const State& seed);
	
	const Gecode::IntSet& get_domain(VariableIdx variable) const { return _domains.at(variable); }
	const Gecode::IntSet& get_delta(VariableIdx variable) const { return _deltas.at(variable); }
	
	void set(VariableIdx variable, ObjectIdx value) {
		_index.at(variable).insert(value);
	}
	
	//! Return the number of state variables handled by the layer
	unsigned width() const { return _domains.size(); }
	
	void rebuild_domains(const std::vector<std::vector<ObjectIdx>>& novel_atoms);
	
	bool contains(const Atom& atom) const;
	
	GecodeRPGLayer(const GecodeRPGLayer& state)  = delete;
	GecodeRPGLayer& operator=(const GecodeRPGLayer& rhs) = delete;
	bool operator==(const GecodeRPGLayer& rhs) = delete;
	
	//! Prints a representation of the state to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const GecodeRPGLayer&  state) { return state.print(os); }
	std::ostream& print(std::ostream& os) const;
	
protected:
	//! A helper
	static std::ostream& print_intsets(std::ostream& os, const ProblemInfo& info, const std::vector<Gecode::IntSet>& intsets);
};

} } // namespaces

