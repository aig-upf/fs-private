
#pragma once

#include <gecode/int.hh>
#include <fs_types.hxx>
#include <utils/index.hxx>


namespace fs0 { class ProblemInfo; class State; class Atom; }

namespace fs0 { namespace gecode {

class ExtensionHandler;

class GecodeRPGLayer {
protected:
	//! The set of possible value for every state variable
	std::vector<Gecode::IntSet> _domains;
	
	//! A vector indexing each atom through its variable
	std::vector<Domain> _index;

	//! The set of variable deltas, i.e. only those values that were newly achieved in the previous RPG layer
	std::vector<Gecode::IntSet> _deltas;
	
	//! The allowed values in the relation that corresponds to every predicate
	std::vector<Gecode::TupleSet> _extensions;
	
	ExtensionHandler& _extension_handler;
	
public:
	explicit GecodeRPGLayer(ExtensionHandler& extension_handler, const State& seed);
	
	GecodeRPGLayer(const GecodeRPGLayer& state)  = delete;
	GecodeRPGLayer& operator=(const GecodeRPGLayer& rhs) = delete;
	bool operator==(const GecodeRPGLayer& rhs) = delete;	
	
	bool is_true(VariableIdx variable) const;
	const std::vector<Gecode::IntSet>& get_domains() const { return _domains; }
	const Gecode::IntSet& get_domain(VariableIdx variable) const { return _domains.at(variable); }
	const Gecode::IntSet& get_delta(VariableIdx variable) const { return _deltas.at(variable); }
	const Gecode::TupleSet& get_extension(unsigned symbol_id) const { return _extensions.at(symbol_id); }
	
	const std::set<unsigned>& get_modified_symbols() const;
	
	//! Returns, for all the atoms in the given index, a set with all those which have NOT been achieved in the current layer.
	std::set<unsigned> unachieved_atoms(const Index<Atom>& atom_idx) const;
	
	//! Signals the advance to the next RPG layer by accumulaing all the given newly supported atoms
	void advance(const std::vector<std::vector<ObjectIdx>>& novel_atoms);
	
	//! Prints a representation of the state to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const GecodeRPGLayer&  state) { return state.print(os); }
	std::ostream& print(std::ostream& os) const;
	
protected:
	//! A helper
	static std::ostream& print_intsets(std::ostream& os, const ProblemInfo& info, const std::vector<Gecode::IntSet>& intsets);
};

} } // namespaces
