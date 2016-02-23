

#include <constraints/gecode/rpg_layer.hxx>
#include <problem.hxx>
#include <state.hxx>
#include <utils/printers/gecode.hxx>
#include <boost/concept_check.hpp>

namespace fs0 { namespace gecode {

//! A small helper class to generate the Gecode tuplesets.
class Extension {
protected:
	std::vector<std::vector<int>> _tuples;
	
public:
	void add_tuple(const std::vector<int> tuple) {
		_tuples.push_back(tuple);
	}
	
	bool is_tautology() const {
		return _tuples.size() == 1 and _tuples[0].empty();
	}
	
	Gecode::TupleSet generate() {
		Gecode::TupleSet ts;
		if (is_tautology()) return ts; // We return an empty extension, since the symbol will be dealt with differently
		
		for (auto& elem:_tuples) {
			ts.add(elem);
		}
		ts.finalize();
		return ts;
	}
};


GecodeRPGLayer::GecodeRPGLayer(const State& seed)
	: _index(seed.numAtoms())
{
	const ProblemInfo& info = Problem::getInfo();
	_domains.reserve(seed.numAtoms());
	_deltas.reserve(seed.numAtoms());
	
	std::vector<Extension> extensions(info.getNumLogicalSymbols());
	
	// Initially, all domains and deltas are set to contain exactly the values from the seed state
	for (unsigned variable = 0; variable < seed.numAtoms(); ++variable) {
		ObjectIdx value = seed.getValue(variable);
		_index[variable].insert(value);
		_domains.push_back(Gecode::IntSet(value, value));
		_deltas.push_back(Gecode::IntSet(value, value));
		
		// Build the predicate symbol extensions
		const auto& vardata = info.getVariableData(variable);
		if (info.isPredicate(vardata.first)) {
			if (value) { // The predicate is true - add the tuple to the extension
				extensions.at(vardata.first).add_tuple(vardata.second);
			}
		}
	}
	
	// Generate the tuplesets
	_predicate_extensions.clear();
	for (auto& generator:extensions) {
		_predicate_extensions.push_back(generator.generate());
	}
}


void GecodeRPGLayer::accumulate(const std::vector<std::vector<ObjectIdx>>& novel_atoms) {
	const ProblemInfo& info = Problem::getInfo();
	assert(novel_atoms.size() == _domains.size());
	
	std::vector<Extension> extensions(info.getNumLogicalSymbols());
	
	for (VariableIdx variable = 0; variable < novel_atoms.size(); ++variable) {
		// Rebuild the delta only with the novel atoms
		const auto& delta = novel_atoms[variable];
		_deltas[variable] = Gecode::IntSet(delta.data(), delta.size());
		
		// Update the index and rebuild the full domain with all indexed atoms
		// An intermediate IntArgs object seems to be necessary, since IntSets do not accept std-like range constructors.
		Domain& domain = _index[variable];
		domain.insert(delta.cbegin(), delta.cend());
		_domains[variable] = Gecode::IntSet(Gecode::IntArgs(domain.cbegin(), domain.cend()));
		
		
		// Build (from scratch, ATM!) the predicate symbol extensions
		const auto& vardata = info.getVariableData(variable);
		if (info.isPredicate(vardata.first)) {
			if (domain.find(1) != domain.end()) { // The predicate is true - add the tuple to the extension
				extensions.at(vardata.first).add_tuple(vardata.second);
			}
		}
		
		// Generate the tuplesets
		_predicate_extensions.clear();
		for (auto& generator:extensions) {
			_predicate_extensions.push_back(generator.generate());
		}
	}
}


std::ostream& GecodeRPGLayer::print(std::ostream& os) const {
	const ProblemInfo& info = Problem::getInfo();
	os << "GecodeRPGLayer:" << std::endl;
	
	os << "Indexed atoms:" << std::endl << "\t";
	for (unsigned i = 0; i < _index.size(); ++i) { // Iterate through all the sets
		os << info.getVariableName(i) << "={";
		for (ObjectIdx value:_index[i]) os << info.getObjectName(i, value) << ",";
		os << "}";
		if (i < _index.size() - 1) os << ", ";
	}
	
	os << std::endl << "IntSets:" << std::endl << "\t";
	print_intsets(os, info, _domains);
	os << std::endl << "Deltas:" << std::endl << "\t";
	print_intsets(os, info, _deltas);
	os << std::endl;
	return os;
}

std::ostream& GecodeRPGLayer::print_intsets(std::ostream& os, const ProblemInfo& info, const std::vector<Gecode::IntSet>& intsets) {
	for (unsigned i = 0; i < intsets.size(); ++i) { // Iterate through all the sets
		os << info.getVariableName(i) << "={";
		for (Gecode::IntSetValues values(intsets.at(i)); values(); ++values) {
			os << info.getObjectName(i, values.val()) << ",";
		}
		os << "}";
		if (i < intsets.size() - 1) os << ", ";
	}
	return os;
}


} } // namespaces
