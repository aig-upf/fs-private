
#pragma once
#include <fs0_types.hxx>
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
	Extension(const TupleIndex& tuple_index) : _tuple_index(tuple_index), _tuples() {}
	
	void add_tuple(TupleIdx tuple) { _tuples.push_back(tuple); }
	
	bool is_tautology() const {
		return _tuples.size() == 1 && _tuple_index.from_index(_tuples[0]).empty();
	}
	
	Gecode::TupleSet generate() const {
		Gecode::TupleSet ts;
		if (is_tautology()) return ts; // We return an empty extension, since the symbol will be dealt with differently
		
		for (TupleIdx index:_tuples) {
			ts.add(_tuple_index.from_index(index));
		}
		ts.finalize();
		return ts;
	}
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
