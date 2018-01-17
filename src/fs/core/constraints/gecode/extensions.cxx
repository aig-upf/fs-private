

#include <fs/core/constraints/gecode/extensions.hxx>
#include <fs/core/problem_info.hxx>
#include <fs/core/state.hxx>
#include <utility>

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


ExtensionHandler::ExtensionHandler(const AtomIndex &tuple_index,
								   std::vector<unsigned> managed) :
	_info(ProblemInfo::getInstance()),
	_tuple_index(tuple_index),
	_extensions(std::vector<Extension>(_info.getNumLogicalSymbols(), Extension(_tuple_index))), // Reset the whole vector
	_managed(std::move(managed))
{}

void ExtensionHandler::reset() {
	_extensions = std::vector<Extension>(_info.getNumLogicalSymbols(), Extension(_tuple_index)); // Reset the whole vector
	advance();
}

void ExtensionHandler::advance() {
}

AtomIdx ExtensionHandler::process_atom(VariableIdx variable, const object_id& value) {
	const auto& tuple_data = _info.getVariableData(variable);
	bool is_predicate = _info.isPredicativeVariable(variable);
	unsigned symbol = tuple_data.first;


	// "False" (i.e. negated) atoms do not result in any additional tuple being added to the extension of the symbol
	if (is_predicate && int(value) == 0) return INVALID_TUPLE;

	AtomIdx index;

	if (is_predicate && int(value) == 1) {
		index = _tuple_index.to_index(tuple_data);

	} else {
		ValueTuple tuple = tuple_data.second; // (implicitly copies)
		tuple.push_back(value);
		index = _tuple_index.to_index(symbol, tuple);
	}

	if (_managed.at(symbol)) {
		_extensions.at(symbol).add_tuple(index);
	}
	return index;
}


void ExtensionHandler::process_tuple(AtomIdx tuple) {
	unsigned symbol = _tuple_index.symbol(tuple);
// 	_modified.insert(symbol_idx);  // Mark the extension as modified
	if (_managed.at(symbol)) {
		_extensions.at(symbol).add_tuple(tuple);
	}
}

void ExtensionHandler::process_delta(VariableIdx variable, const std::vector<object_id>& delta) {
	for (const object_id& value:delta) process_atom(variable, value);
}

std::vector<Gecode::TupleSet> ExtensionHandler::generate_extensions() const {
	std::vector<Gecode::TupleSet> result;
	
	for (unsigned symbol = 0; symbol < _extensions.size(); ++symbol) {
		auto& generator = _extensions[symbol];
		if (_managed.at(symbol)) {
			result.push_back(generator.generate());
		} else {
			result.emplace_back();
		}
	}
	return result;
}

Gecode::TupleSet ExtensionHandler::generate_extension(unsigned symbol) const {
	auto& generator = _extensions[symbol];
	return generator.generate();
}



StateBasedExtensionHandler::StateBasedExtensionHandler(const AtomIndex& tuple_index) :
		StateBasedExtensionHandler(tuple_index, std::vector<unsigned>(ProblemInfo::getInstance().getNumLogicalSymbols(), 1))
{}

StateBasedExtensionHandler::StateBasedExtensionHandler(const AtomIndex& tuple_index, std::vector<unsigned> managed) :
		_info(ProblemInfo::getInstance()),
		_tuple_index(tuple_index),
		_tuplesets(_info.getNumLogicalSymbols()),
		_managed(std::move(managed))
{}



void StateBasedExtensionHandler::process(const State& state) {

//    Gecode::TupleSet test;
//    std::cout << "[0] We have " << test.tuples() << " tuples" << std::endl;

//    std::cout << "We have " << _tuplesets[0].tuples() << " tuples" << std::endl;
	for (unsigned variable = 0, n = state.numAtoms(); variable < n; ++variable) {
		unsigned symbol = _tuple_index.var_to_symbol(variable);
		assert(symbol < ProblemInfo::getInstance().getNumLogicalSymbols());
		if (!_managed[symbol]) continue;

		// "False" (i.e. negated) atoms do not result in any additional tuple being added to the extension of the symbol
		const auto val = state.getValue(variable);
		if (_info.isPredicativeVariable(variable) && int(val) == 0) continue;

		const ValueTuple& values = _tuple_index.to_tuple(variable, val);
		_tuplesets[symbol].add(fs0::values<int>(values, ObjectTable::EMPTY_TABLE));
	}

	for (auto& ts:_tuplesets) ts.finalize();
}

const Gecode::TupleSet& StateBasedExtensionHandler::retrieve(unsigned symbol) const {
	return _tuplesets[symbol];
}


	} } // namespaces
