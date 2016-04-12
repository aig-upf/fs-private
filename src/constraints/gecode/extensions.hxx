
#pragma once
#include <fs_types.hxx>
#include <utils/tuple_index.hxx>
#include <gecode/int.hh>

namespace fs0 { class ProblemInfo; }

namespace fs0 { namespace gecode {

//! A small helper class to generate the Gecode tuplesets.
class Extension {
protected:
	const TupleIndex& _tuple_index;
	
	std::vector<TupleIdx> _tuples;
	
public:
	Extension(const TupleIndex& tuple_index);
	
	void add_tuple(TupleIdx tuple);
	
	bool is_tautology() const;
	
	Gecode::TupleSet generate() const;
};

class ExtensionHandler {
protected:
	const ProblemInfo& _info;
	
	const TupleIndex& _tuple_index;
	
	//! _extensions[i] contains the extension of logical symbol 'i'
	std::vector<Extension> _extensions;
	
	//! _modified[i] is true iff the denotation of logical symbol 'i' changed on the last layer
	std::set<unsigned> _modified;
public:
	ExtensionHandler(const TupleIndex& tuple_index);
	
	void reset();
	
	//! Processes an atom and returns the equivalent extension tuple.
	TupleIdx process_atom(VariableIdx variable, ObjectIdx value);
	
	void process_tuple(TupleIdx tuple);

	// TODO REMOVE?
	void process_delta(VariableIdx variable, const std::vector<ObjectIdx>& delta);
	
	void advance();
	
	const std::set<unsigned>& get_modified_symbols() const { return _modified; }
	
	std::vector<Gecode::TupleSet> generate_extensions() const;
	
};

} } // namespaces
