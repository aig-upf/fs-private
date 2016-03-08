
#pragma once
#include <fs0_types.hxx>
#include <gecode/int.hh>

namespace fs0 { class ProblemInfo; }

namespace fs0 { namespace gecode {

//! A small helper class to generate the Gecode tuplesets.
class Extension {
protected:
	std::vector<std::vector<int>> _tuples;
	
public:
	void add_tuple(const std::vector<int>& tuple) { _tuples.push_back(tuple); }
	void add_tuple(std::vector<int>&& tuple) { _tuples.push_back(std::move(tuple)); }
	
	bool is_tautology() const {
		return _tuples.size() == 1 and _tuples[0].empty();
	}
	
	Gecode::TupleSet generate() const {
		Gecode::TupleSet ts;
		if (is_tautology()) return ts; // We return an empty extension, since the symbol will be dealt with differently
		
		for (auto& elem:_tuples) {
			ts.add(elem);
		}
		ts.finalize();
		return ts;
	}
};

class ExtensionHandler {
protected:
	const ProblemInfo& _info;
	
	//! _extensions[i] contains the extension of logical symbol 'i'
	std::vector<Extension> _extensions;
	
	//! _modified[i] is true iff the denotation of logical symbol 'i' changed on the last layer
	std::set<unsigned> _modified;
public:
	ExtensionHandler();
	
	void reset();
	
	void process_atom(VariableIdx variable, ObjectIdx value);
	
	void process_delta(VariableIdx variable, const std::vector<ObjectIdx>& delta);
	
	void advance();
	
	const std::set<unsigned>& get_modified_symbols() const { return _modified; }
	
	std::vector<Gecode::TupleSet> generate_extensions() const;
	
};

} } // namespaces
