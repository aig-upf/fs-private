
#include <cassert>
#include <algorithm>

#include <heuristics/novelty/multivalued_evaluator.hxx>


namespace fs0 {

ValuesTuple::ValuesTuple( std::size_t sz, bool preallocate ) {
	if (preallocate) elements.resize( sz );
	else elements.reserve( sz );
}


std::ostream&
operator<<(std::ostream& stream, const ValuesTuple& t) {
	stream << "[";
	for (const auto& elem : t.elements) {
		stream << "(" << elem.first << "," << elem.second << ")";
	}
	stream << "]";
	return stream;
}

std::size_t ValuesTuple::Hasher::operator()(const ValuesTuple& tuple) const {
	return boost::hash_range(tuple.elements.begin(), tuple.elements.end());
}


unsigned
MultivaluedNoveltyEvaluator::evaluate(const FeatureValuation& current, const std::vector<bool>& novel) {
	assert(!current.empty());
	assert(current.size() == novel.size());

	unsigned state_novelty = evaluate_width_1_tuples(current, novel);

	for (unsigned min_novelty = 2; min_novelty <= _max_novelty; ++min_novelty) {

		TupleIterator it(min_novelty, current, novel);
		
		ValuesTuple tuple(min_novelty);
		bool updated_tables = false;
		while ( it.next(tuple) ) {
			auto result = _tables[min_novelty].insert(tuple);
			updated_tables |= result.second;
		}
		
		if ( updated_tables && min_novelty < state_novelty) {
			state_novelty = min_novelty;
		}
	}

	return state_novelty;
}

} // namespaces
