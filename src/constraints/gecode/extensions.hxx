
#pragma once
#include <fs_types.hxx>
#include <utils/tuple_index.hxx>
#include <gecode/int.hh>

namespace fs0 { class ProblemInfo; }

namespace fs0 { namespace gecode {

//! A small helper class to generate the Gecode tuplesets.
class Extension {
protected:
	const AtomIndex& _tuple_index;
	
	std::vector<AtomIdx> _tuples;
	
public:
	Extension(const AtomIndex& tuple_index);
	
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
	std::vector<bool> _managed;
	
	//! _modified[i] is true iff the denotation of logical symbol 'i' changed on the last layer
// 	std::set<unsigned> _modified;
public:
	ExtensionHandler(const AtomIndex& tuple_index, std::vector<bool> managed);
	
	//! Default copy constructors and assignment operators
	ExtensionHandler(const ExtensionHandler& other) = default;
	ExtensionHandler(ExtensionHandler&& other) = default;
	ExtensionHandler& operator=(const ExtensionHandler& other) = default;
	ExtensionHandler& operator=(ExtensionHandler&& other) = default;
	
	void reset();
	
	//! Processes an atom and returns the equivalent extension tuple.
	AtomIdx process_atom(VariableIdx variable, ObjectIdx value);
	
	void process_tuple(AtomIdx tuple);

	// TODO REMOVE?
	void process_delta(VariableIdx variable, const std::vector<ObjectIdx>& delta);
	
	void advance();
	
// 	const std::set<unsigned>& get_modified_symbols() const { return _modified; }
	
	std::vector<Gecode::TupleSet> generate_extensions() const;
	
	Gecode::TupleSet generate_extension(unsigned symbol_id) const;
};

} } // namespaces
