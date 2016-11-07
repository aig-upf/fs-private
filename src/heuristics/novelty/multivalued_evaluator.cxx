
#include <cassert>
#include <algorithm>

#include <heuristics/novelty/multivalued_evaluator.hxx>


namespace fs0 {

ValuesTuple::ValuesTuple(std::size_t sz) {
	elements.reserve(sz*2);
}

std::ostream& 
ValuesTuple::print(std::ostream& os) const {
	assert(elements.size() % 2 == 0);
	os << "[";
	for (unsigned i = 0; i < elements.size(); i = i+2) {
		os << "(" << elements[i] << "," << elements[i+1] << ")";
	}
	os << "]";
	return os;
}

std::size_t ValuesTuple::Hasher::operator()(const ValuesTuple& tuple) const {
	assert(tuple._check_ordered());
	return boost::hash_range(tuple.elements.begin(), tuple.elements.end());
}


unsigned
MultivaluedNoveltyEvaluator::evaluate_width_1_tuples(const FeatureValuation& current, const std::vector<bool>& novel) {
	unsigned state_novelty = current.size() + 1;
	
	for (unsigned i = 0; i < current.size(); ++i) {
		if (!novel[i]) continue; // Surely the value won't be new
		
		auto res = _width_1_tuples.insert(std::make_pair(i, current[i]));
		if (!res.second) continue; // This tuple was already accounted for
		
		state_novelty = 1; // Otherwise, the value must be new and hence the novelty of the state 1
	}
	return state_novelty;
}
	
unsigned
MultivaluedNoveltyEvaluator::evaluate(const FeatureValuation& current, const std::vector<bool>& novel) {
	assert(!current.empty());
	assert(current.size() == novel.size());

	unsigned state_novelty = evaluate_width_1_tuples(current, novel);

	for (unsigned min_novelty = 2; min_novelty <= _max_novelty; ++min_novelty) {

		bool updated_tables = false;
		TupleIterator it(min_novelty, current, novel);
		
		while (!it.ended()) {
			ValuesTuple tuple = it.next();
			auto result = _tables[min_novelty].insert(tuple);
			updated_tables |= result.second;
		}
		
		if (updated_tables && min_novelty < state_novelty) {
			state_novelty = min_novelty;
		}
	}

	return state_novelty;
}

} // namespaces
