

#include <constraints/gecode/rpg_layer.hxx>
#include <problem.hxx>
#include <state.hxx>
#include <utils/printers/gecode.hxx>
#include <constraints/gecode/extensions.hxx>

namespace fs0 { namespace gecode {

GecodeRPGLayer::GecodeRPGLayer(ExtensionHandler& extension_handler, const State& seed)
	: _index(seed.numAtoms()),
	  _extension_handler(extension_handler)
{
	const ProblemInfo& info = Problem::getInfo();
	_domains.reserve(seed.numAtoms());
	_deltas.reserve(seed.numAtoms());
	
	_extension_handler.reset();
	
	// Initially, all domains and deltas are set to contain exactly the values from the seed state
	for (unsigned variable = 0; variable < seed.numAtoms(); ++variable) {
		ObjectIdx value = seed.getValue(variable);
		
		if (info.isPredicativeVariable(variable) && value == 0) { // TODO This check is expensive and should be optimized out
			_domains.push_back(Gecode::IntSet());
			_deltas.push_back(Gecode::IntSet());
		} else {
			_index[variable].insert(value);
			_domains.push_back(Gecode::IntSet(value, value));
			_deltas.push_back(Gecode::IntSet(value, value));
			_extension_handler.process_atom(variable, value);
		}
	}
	
	_extensions = _extension_handler.generate_extensions();
}

const std::set<unsigned>& GecodeRPGLayer::get_modified_symbols() const {
	return _extension_handler.get_modified_symbols();
}

void GecodeRPGLayer::advance(const std::vector<std::vector<ObjectIdx>>& novel_atoms) {
	assert(novel_atoms.size() == _domains.size());
	
	_extension_handler.advance();
	
	for (VariableIdx variable = 0; variable < novel_atoms.size(); ++variable) {
		// Rebuild the delta only with the novel atoms
		const auto& delta = novel_atoms[variable];
		_deltas[variable] = Gecode::IntSet(delta.data(), delta.size());
		
		// Update the index and rebuild the full domain with all indexed atoms
		// An intermediate IntArgs object seems to be necessary, since IntSets do not accept std-like range constructors.
		Domain& domain = _index[variable];
		domain.insert(delta.cbegin(), delta.cend());
		_domains[variable] = Gecode::IntSet(Gecode::IntArgs(domain.cbegin(), domain.cend()));
		
		
		// Add the new atoms to the symbol extensions
		_extension_handler.process_delta(variable, delta);
	}
	
	_extensions = _extension_handler.generate_extensions();
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

std::set<unsigned> GecodeRPGLayer::unachieved_atoms(const Index<Atom>& atom_idx) const {
	std::set<unsigned> unachieved;
	
	for (const Atom& atom:atom_idx.elements()) {
		const auto& domain = _index.at(atom.getVariable());
		if (domain.find(atom.getValue()) == domain.end()) {
			unachieved.insert(atom_idx.index(atom));
		}
	}
	return unachieved;
}

} } // namespaces
