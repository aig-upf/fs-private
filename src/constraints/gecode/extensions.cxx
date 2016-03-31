

#include <constraints/gecode/extensions.hxx>
#include <problem.hxx>
#include <state.hxx>

namespace fs0 { namespace gecode {

ExtensionHandler::ExtensionHandler(const TupleIndex& tuple_index) :
	_info(Problem::getInfo()),
	_tuple_index(tuple_index)
{}

void ExtensionHandler::reset() {
	_extensions = std::vector<Extension>(_info.getNumLogicalSymbols(), Extension(_tuple_index)); // Reset the whole vector
	advance();
}

void ExtensionHandler::advance() {
	_modified.clear(); // Initially all symbols are untouched
}

TupleIdx ExtensionHandler::process_atom(VariableIdx variable, ObjectIdx value) {
	const auto& tuple_data = _info.getVariableData(variable); // TODO - MOVE FROM PROBLEM INFO INTO SOME PERFORMANT INDEX
	unsigned symbol = tuple_data.first;
	bool is_predicate = _info.isPredicativeVariable(variable); // TODO - MOVE FROM PROBLEM INFO INTO SOME PERFORMANT INDEX
	Extension& extension = _extensions.at(symbol);
	_modified.insert(symbol);  // Mark the extension as modified
	
	if (is_predicate && value == 1) {
		TupleIdx index = _tuple_index.to_index(tuple_data);
		extension.add_tuple(index);
		return (int) index;
	}
	
	if (!is_predicate) {
		ValueTuple tuple = tuple_data.second; // (implicitly copies)
		tuple.push_back(value);
		TupleIdx index = _tuple_index.to_index(symbol, tuple);
		extension.add_tuple(index);
		return (int) index;
	}
	
	return INVALID_TUPLE;
}

void ExtensionHandler::process_tuple(TupleIdx tuple) {
	unsigned symbol_idx = _tuple_index.symbol(tuple);
	Extension& extension = _extensions.at(symbol_idx);
	_modified.insert(symbol_idx);  // Mark the extension as modified
	extension.add_tuple(tuple);
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
