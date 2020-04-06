
#pragma once
#include <fs/core/fs_types.hxx>
#include <fs/core/utils/atom_index.hxx>
#include <gecode/int.hh>

namespace fs0 { class ProblemInfo; class State; }

namespace fs0::gecode {

//! A small helper class to generate the Gecode tuplesets.
class Extension {
protected:
	const AtomIndex& _tuple_index;
	
	std::vector<AtomIdx> _tuples;
	
public:
	explicit Extension(const AtomIndex& tuple_index);
	
	void add_tuple(AtomIdx tuple);
	
	bool is_tautology() const;
	
	Gecode::TupleSet generate() const;
};

class ExtensionHandler {
protected:
	const ProblemInfo& _info;
	
	const AtomIndex& _tuple_index;
	
	//! _extensions[i] contains the extension of logical symbol 'i'
	std::vector<Extension> _extensions;

	//! _managed[i] tells us whether we need to manage the extension of logical symbol 'i' or not.
	std::vector<unsigned> _managed;

public:
	ExtensionHandler(const AtomIndex& tuple_index, std::vector<unsigned> managed);
	
	//! Default copy constructors and assignment operators
	ExtensionHandler(const ExtensionHandler& other) = default;
	ExtensionHandler(ExtensionHandler&& other) = default;
	ExtensionHandler& operator=(const ExtensionHandler& other) = delete;
	ExtensionHandler& operator=(ExtensionHandler&& other) = delete;
	
	void reset();
	
	//! Processes an atom and returns the equivalent extension tuple.
	AtomIdx process_atom(VariableIdx variable, const object_id& value);
	
	void process_tuple(AtomIdx tuple);

	void advance();
	
// 	const std::set<unsigned>& get_modified_symbols() const { return _modified; }
	
	std::vector<Gecode::TupleSet> generate_extensions() const;
	
	Gecode::TupleSet generate_extension(unsigned symbol_id) const;
};




class StateBasedExtensionHandler {
protected:
	const ProblemInfo& _info;

	const AtomIndex& _tuple_index;

	//!
	std::vector<Gecode::TupleSet> _tuplesets;

	//! _managed[i] tells us whether we need to manage the extension of logical symbol 'i' or not.
	std::vector<unsigned> _managed;

public:
	//! A handler managing all symbols
	StateBasedExtensionHandler(const AtomIndex& tuple_index, const State& state);

	//! A handler managing only the selected symbols
	StateBasedExtensionHandler(const AtomIndex& tuple_index, std::vector<unsigned> managed, const State& state);

	//! Default copy constructors and assignment operators
	StateBasedExtensionHandler(const StateBasedExtensionHandler&) = default;
	StateBasedExtensionHandler(StateBasedExtensionHandler&&) = default;
	StateBasedExtensionHandler& operator=(const StateBasedExtensionHandler&) = delete;
	StateBasedExtensionHandler& operator=(StateBasedExtensionHandler&&) = delete;

	const Gecode::TupleSet& retrieve(unsigned symbol_id) const;
};
} // namespaces
