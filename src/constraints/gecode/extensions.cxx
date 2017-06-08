

#include <constraints/gecode/extensions.hxx>
#include <problem_info.hxx>
#include <state.hxx>

namespace fs0 { namespace gecode {

Extension::Extension(const AtomIndex& tuple_index) : _tuple_index(tuple_index), _tuples() {}

bool Extension::is_tautology() const {
	return _tuples.size() == 1 && _tuple_index.to_tuple(_tuples[0]).empty();
}

void Extension::add_tuple(AtomIdx tuple) {
// 	assert(std::find(_tuples.begin(), _tuples.end(), tuple) == _tuples.end()); // This is an expensive assert
	_tuples.push_back(tuple);
}

Gecode::TupleSet Extension::generate() const {
	Gecode::TupleSet ts;
	if (is_tautology()) return ts; // We return an empty extension, since the symbol will be dealt with differently
	
	for (AtomIdx index:_tuples) {
		const ValueTuple& objects = _tuple_index.to_tuple(index);
		ts.add(values<int>(objects, ObjectTable::EMPTY_TABLE));
	}
	ts.finalize();
	return ts;
}


ExtensionHandler::ExtensionHandler(const AtomIndex& tuple_index, std::vector<bool> managed) :
	_info(ProblemInfo::getInstance()),
	_tuple_index(tuple_index),
	_extensions(std::vector<Extension>(_info.getNumLogicalSymbols(), Extension(_tuple_index))), // Reset the whole vector
	_managed(managed)
{}

void ExtensionHandler::reset() {
	_extensions = std::vector<Extension>(_info.getNumLogicalSymbols(), Extension(_tuple_index)); // Reset the whole vector
	advance();
}

void ExtensionHandler::advance() {
// 	_modified.clear(); // Initially all symbols are untouched
}

AtomIdx ExtensionHandler::process_atom(VariableIdx variable, object_id value) {
	const auto& tuple_data = _info.getVariableData(variable); // TODO - MOVE FROM PROBLEM INFO INTO SOME PERFORMANT INDEX
	unsigned symbol = tuple_data.first;
	bool managed = _managed.at(symbol);
	bool is_predicate = _info.isPredicativeVariable(variable); // TODO - MOVE FROM PROBLEM INFO INTO SOME PERFORMANT INDEX
	Extension& extension = _extensions.at(symbol);
// 	_modified.insert(symbol);  // Mark the extension as modified
	
	if (is_predicate && int(value) == 1) {
		AtomIdx index = _tuple_index.to_index(tuple_data);
		if (managed) {
			extension.add_tuple(index);
		}
		return index;
	}
	
	if (!is_predicate) {
		ValueTuple tuple = tuple_data.second; // (implicitly copies)
		tuple.push_back(value);
		AtomIdx index = _tuple_index.to_index(symbol, tuple);
		if (managed) {
			extension.add_tuple(index);
		}
		return index;
	}
	
	return INVALID_TUPLE;
}

void ExtensionHandler::process_tuple(AtomIdx tuple) {
	unsigned symbol = _tuple_index.symbol(tuple);
// 	_modified.insert(symbol_idx);  // Mark the extension as modified
	if (_managed.at(symbol)) {
		_extensions.at(symbol).add_tuple(tuple);
	}
}

void ExtensionHandler::process_delta(VariableIdx variable, const std::vector<object_id>& delta) {
	for (object_id value:delta) process_atom(variable, value);
}


std::vector<Gecode::TupleSet> ExtensionHandler::generate_extensions() const {
	std::vector<Gecode::TupleSet> result;
	
	for (unsigned symbol = 0; symbol < _extensions.size(); ++symbol) {
		auto& generator = _extensions[symbol];
		if (_managed.at(symbol)) {
			result.push_back(generator.generate());
		} else {
			result.push_back(Gecode::TupleSet());
		}
	}
	return result;
}

Gecode::TupleSet ExtensionHandler::generate_extension(unsigned symbol) const {
	auto& generator = _extensions[symbol];
	return generator.generate();
}

} } // namespaces
