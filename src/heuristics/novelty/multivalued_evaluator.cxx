
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

std::size_t MultivaluedNoveltyEvaluator::Width2TupleHasher::operator()(const Width2Tuple& tuple) const {
	assert(std::get<0>(tuple) < std::get<2>(tuple));
	std::size_t seed = 0;
	boost::hash_combine(seed, std::get<0>(tuple));
	boost::hash_combine(seed, std::get<1>(tuple));
	boost::hash_combine(seed, std::get<2>(tuple));
	boost::hash_combine(seed, std::get<3>(tuple));
	return seed;
}


unsigned
MultivaluedNoveltyEvaluator::evaluate_width_1_tuples(const FeatureValuation& current, const std::vector<unsigned>& novel) {
	unsigned state_novelty = current.size() + 1;
	
	for (unsigned idx:novel) {
		auto res = _width_1_tuples.insert(std::make_pair(idx, current[idx]));
		if (res.second) state_novelty = 1; // The tuple is new, hence the novelty of the state is 1
	}
	return state_novelty;
}

unsigned
MultivaluedNoveltyEvaluator::evaluate_width_2_tuples(unsigned state_novelty, const FeatureValuation& current, const std::vector<unsigned>& novel) {
	
	for (unsigned i = 0; i < novel.size(); ++i) {
		unsigned novel_idx = novel[i];
		int novel_val = current[novel_idx];
		
		for (unsigned j = 0; j < novel_idx; ++j) {
			auto res = _width_2_tuples.insert(std::make_tuple(j, current[j], novel_idx, novel_val));
			if (res.second && state_novelty > 2) state_novelty = 2; // The tuple was new, hence the novelty is not more than 2
		}
		
		for (unsigned j = novel_idx+1; j < current.size(); ++j) {
			auto res = _width_2_tuples.insert(std::make_tuple(novel_idx, novel_val, j, current[j]));
			if (res.second && state_novelty > 2) state_novelty = 2; // The tuple was new, hence the novelty is not more than 2
		}
	}

	return state_novelty;
}

unsigned
MultivaluedNoveltyEvaluator::evaluate(const FeatureValuation& current, const std::vector<unsigned>& novel) {
	assert(!current.empty());

	unsigned state_novelty = evaluate_width_1_tuples(current, novel);
	if (_max_novelty <= 1) return state_novelty;
	
	state_novelty = evaluate_width_2_tuples(state_novelty, current, novel);
	if (_max_novelty <= 2) return state_novelty;
	
	
	std::vector<bool> novel_idx(current.size(), false);
	for (unsigned idx:novel) { novel_idx[idx] = true;}

	for (unsigned novelty = 3; novelty <= _max_novelty; ++novelty) {

		bool updated_tables = false;
		TupleIterator it(novelty, current, novel_idx);
		
		while (!it.ended()) {
			auto result = _tables[novelty].insert(it.next());
			updated_tables |= result.second;
		}
		
		if (updated_tables && novelty < state_novelty) {
			state_novelty = novelty;
		}
	}

	return state_novelty;
}

} // namespaces
