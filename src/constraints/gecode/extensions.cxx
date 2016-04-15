

#include <constraints/gecode/extensions.hxx>
#include <problem_info.hxx>
#include <state.hxx>

namespace fs0 { namespace gecode {

Extension::Extension(const TupleIndex& tuple_index) : _tuple_index(tuple_index), _tuples() {}

bool Extension::is_tautology() const {
	return _tuples.size() == 1 && _tuple_index.to_tuple(_tuples[0]).empty();
}

void Extension::add_tuple(TupleIdx tuple) {
// 	assert(std::find(_tuples.begin(), _tuples.end(), tuple) == _tuples.end()); // This is an expensive assert
	_tuples.push_back(tuple);
}

Gecode::TupleSet Extension::generate() const {
	Gecode::TupleSet ts;
	if (is_tautology()) return ts; // We return an empty extension, since the symbol will be dealt with differently
	
	for (TupleIdx index:_tuples) {
		ts.add(_tuple_index.to_tuple(index));
	}
	ts.finalize();
	return ts;
}


ExtensionHandler::ExtensionHandler(const TupleIndex& tuple_index) :
	_info(ProblemInfo::getInstance()),
	_tuple_index(tuple_index),
	_managed(_info.getNumLogicalSymbols(), true) // By default we manage all symbols
{}

void ExtensionHandler::reset() {
	_extensions = std::vector<Extension>(_info.getNumLogicalSymbols(), Extension(_tuple_index)); // Reset the whole vector
	advance();
}

void ExtensionHandler::advance() {
// 	_modified.clear(); // Initially all symbols are untouched
}

TupleIdx ExtensionHandler::process_atom(VariableIdx variable, ObjectIdx value) {
	const auto& tuple_data = _info.getVariableData(variable); // TODO - MOVE FROM PROBLEM INFO INTO SOME PERFORMANT INDEX
	unsigned symbol = tuple_data.first;
	bool is_predicate = _info.isPredicativeVariable(variable); // TODO - MOVE FROM PROBLEM INFO INTO SOME PERFORMANT INDEX
	Extension& extension = _extensions.at(symbol);
// 	_modified.insert(symbol);  // Mark the extension as modified
	
	if (is_predicate && value == 1) {
		TupleIdx index = _tuple_index.to_index(tuple_data);
		extension.add_tuple(index);
		return index;
	}
	
	if (!is_predicate) {
		ValueTuple tuple = tuple_data.second; // (implicitly copies)
		tuple.push_back(value);
		TupleIdx index = _tuple_index.to_index(symbol, tuple);
		extension.add_tuple(index);
		return index;
	}
	
	return INVALID_TUPLE;
}

void ExtensionHandler::process_tuple(TupleIdx tuple) {
	unsigned symbol_idx = _tuple_index.symbol(tuple);
// 	_modified.insert(symbol_idx);  // Mark the extension as modified
	_extensions.at(symbol_idx).add_tuple(tuple);
}

void ExtensionHandler::process_delta(VariableIdx variable, const std::vector<ObjectIdx>& delta) {
	for (ObjectIdx value:delta) process_atom(variable, value);
}


std::vector<Gecode::TupleSet> ExtensionHandler::generate_extensions() const {
	std::vector<Gecode::TupleSet> result;
	for (auto& generator:_extensions) {
		result.push_back(generator.generate());
	}
	return result;
}

} } // namespaces
