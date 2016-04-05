
#include <utils/tuple_index.hxx>
#include <actions/actions.hxx>
#include <problem.hxx>
#include <state.hxx>
#include <relaxed_state.hxx>

namespace fs0 {

TupleIndex::TupleIndex(const ProblemInfo& info) :
	_inverse(info.getNumLogicalSymbols())
{
	std::vector<std::vector<ValueTuple>> tuples_by_symbol = compute_all_reachable_tuples(info);
	
	std::vector<std::pair<unsigned, unsigned>> symbol_ranges;
	
	unsigned idx = 0;
	for (unsigned symbol = 0; symbol < info.getNumLogicalSymbols(); ++symbol) {
		std::pair<unsigned, unsigned> range{idx, 0};
		
		for (const ValueTuple& tuple:tuples_by_symbol.at(symbol)) {
			// For each symbol, we process all derived tuples and keep information
			// about the index range of the tuples that correspond to that symbol.
			
			ValueTuple copy(tuple);
			
			ObjectIdx value = 1;
			if (!info.isPredicate(symbol)) {
				value = copy[copy.size() - 1];
				copy.erase(copy.end() - 1);
			}
			
			VariableIdx variable = info.resolveStateVariable(symbol, copy);
			add(symbol, tuple, idx, Atom(variable, value));
			idx++;
		}
		
		range.second = idx - 1;
		symbol_ranges.push_back(range);
	}
	
}

void TupleIndex::add(unsigned symbol, const ValueTuple& tuple, unsigned idx, const Atom& atom) {
	assert(_index.size() == idx);
	_index.push_back(tuple);
	
	assert(_symbol_index.size() == idx);
	_symbol_index.push_back(symbol);
	
// 	_inverse.insert(std::make_pair(std::make_pair(symbol, tuple), idx));
	_inverse.at(symbol).insert(std::make_pair(tuple, idx));
	
	assert(_tuple_atoms.size() == idx);
	_tuple_atoms.push_back(atom);
	
	_tuple_atoms_inv.insert(std::make_pair(atom, idx));
}

// TODO - We should be applying some reachability analysis here to prune out tuples that will never be reachable at all.
std::vector<std::vector<ValueTuple>> TupleIndex::compute_all_reachable_tuples(const ProblemInfo& info) {
	std::vector<std::vector<ValueTuple>> tuples_by_symbol(info.getNumLogicalSymbols());

	for (VariableIdx var = 0; var < info.getNumVariables(); ++var) {
		const auto& data = info.getVariableData(var);
		auto& symbol_tuples = tuples_by_symbol.at(data.first); // The tupleset corresponding to the symbol index
		
		if (info.isPredicativeVariable(var)) {
			symbol_tuples.push_back(data.second); // We're just interested in the non-negated atom
		
			
		} else { // A function symbol
			for (ObjectIdx value:info.getVariableObjects(var)) {
				std::vector<int> arguments(data.second); // Copy the vector
				arguments.push_back(value);
				symbol_tuples.push_back(std::move(arguments)); 
			}
		}
	}
	return tuples_by_symbol;
}

} // namespaces
