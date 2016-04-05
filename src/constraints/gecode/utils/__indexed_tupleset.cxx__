

#include <constraints/gecode/utils/indexed_tupleset.hxx>

namespace fs0 { namespace gecode {

IndexedTupleset::IndexedTupleset(const TupleVector& tuples) :
	_tuples(index_tuples(tuples)),
	_tupleset(compute_tupleset(_tuples))
{}

Gecode::TupleSet IndexedTupleset::get_tupleset() const { return _tupleset; }

bool IndexedTupleset::empty() const { return _tuples.empty(); }

unsigned IndexedTupleset::size() const { return _tuples.size(); }
	
bool IndexedTupleset::contains(const ValueTuple& tuple) const { return _tuples.find(tuple) != _tuples.end(); }

unsigned IndexedTupleset::get_index(const ValueTuple& tuple) const {
	auto it = _tuples.find(tuple);
	assert(it != _tuples.end());
	return it->second;
}

std::map<ValueTuple, unsigned> IndexedTupleset::index_tuples(const TupleVector& tuples) {
	std::map<ValueTuple, unsigned> index;
	for (unsigned i = 0; i < tuples.size(); ++i) {
		const auto& tuple = tuples[i];
		auto it = index.find(tuple);
		if (it != index.end()) throw std::runtime_error("Duplicate tuple");
		index.insert(it, std::make_pair(tuple, i));
	}
	return index;
}

Gecode::TupleSet IndexedTupleset::compute_tupleset(const std::map<ValueTuple, unsigned>& tuple_index) {
	Gecode::TupleSet ts;
	for (auto elem:tuple_index) {
		ValueTuple indexed(elem.first); // Copy the tuple
		indexed.push_back(elem.second); // append the index
		ts.add(indexed);
	}
	ts.finalize();
	return ts;
}


} } // namespaces
