

#include <fs/core/constraints/gecode/extensions.hxx>
#include <fs/core/problem_info.hxx>
#include <fs/core/state.hxx>
#include <utility>

namespace fs0::gecode {

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
        if (!(bool) ts) ts.init(objects.size());
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
	if (is_predicate && unsigned(value) == 0) return INVALID_TUPLE;

	AtomIdx index;

	if (is_predicate && unsigned(value) == 1) {
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

std::vector<Gecode::TupleSet> ExtensionHandler::generate_extensions() const {
	std::vector<Gecode::TupleSet> result;
	
	for (std::size_t symbol = 0; symbol < _extensions.size(); ++symbol) {
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



StateBasedExtensionHandler::StateBasedExtensionHandler(const AtomIndex& tuple_index, const State& state) :
		StateBasedExtensionHandler(tuple_index, std::vector<unsigned>(ProblemInfo::getInstance().getNumLogicalSymbols(), 1), state)
{}

StateBasedExtensionHandler::StateBasedExtensionHandler(const AtomIndex& tuple_index, std::vector<unsigned> managed, const State& state) :
		_info(ProblemInfo::getInstance()),
		_tuple_index(tuple_index),
		_tuplesets(),
		_managed(std::move(managed))
{
    auto nsymbols = _info.getNumLogicalSymbols();

    // First initialize the vector of tuplesets with the appropriate arities
    for (unsigned i = 0; i < nsymbols; ++i) {
        if (!_managed[i]) {
            _tuplesets.emplace_back(0); // We won't use these
            continue;
        }
        _tuplesets.emplace_back(_info.getSymbolData(i).getArity());
    }

    // Then map the state atoms into the tuples corresponding to each symbol
    for (unsigned variable = 0, n = state.numAtoms(); variable < n; ++variable) {
        unsigned symbol = _tuple_index.var_to_symbol(variable);
        assert(symbol < nsymbols);
        if (!_managed[symbol]) continue;

        // "False" (i.e. negated) atoms do not result in any additional tuple being added to the extension of the symbol
        const auto val = state.getValue(variable);
        if (_info.isPredicativeVariable(variable) && val == object_id::FALSE) continue;

        const ValueTuple& values = _tuple_index.to_tuple(variable, val);

        auto& tset = _tuplesets[symbol];
        assert((std::size_t) tset.arity() == values.size());
        tset.add(fs0::values<int>(values, ObjectTable::EMPTY_TABLE));
    }

    // And "finalize" the tuplesets, a requirement for Gecode to work correctly on them
    for (auto& ts:_tuplesets) ts.finalize();
}

const Gecode::TupleSet& StateBasedExtensionHandler::retrieve(unsigned symbol) const {
	return _tuplesets[symbol];
}


	} // namespaces
