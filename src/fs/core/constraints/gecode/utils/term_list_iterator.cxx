

#include <fs/core/problem_info.hxx>
#include <fs/core/constraints/gecode/utils/term_list_iterator.hxx>
#include <fs/core/utils/cartesian_iterator.hxx>
#include <fs/core/languages/fstrips/terms.hxx>
#include <fs/core/languages/fstrips/operations.hxx>


namespace fs0 { namespace gecode {

term_list_iterator::term_list_iterator(const std::vector<const fs::Term*>& subterms) :
	_subterms(subterms),
	_iterator(new utils::cartesian_iterator(compute_possible_values(subterms)))
{}

term_list_iterator::~term_list_iterator() {
	for (auto vector:_temporary_vectors) delete vector;
	delete _iterator;
}

std::vector<const std::vector<object_id>*> term_list_iterator::compute_possible_values(const std::vector<const fs::Term*>& subterms) {
	const ProblemInfo& info = ProblemInfo::getInstance();
	
	// possible_values[i] will be a vector (pointer) with all possible values for the i-th subterm
	// of the given nested fluent, __according to the signature of the fluent function declaration__
	// But we can optimize this in at least two ways:
	//   - If an actual subterm is a constant, there is no need to consider all possible values for that positional argument
	//   - If an actual subterm is a bound variable, we can only consider the values for the concrete type of the variable, which
	//     might be different (i.e. a subtype, to be more specific) from the generic type of the function declaration.
	// As an example, consider a function f(thing, thing): int
	// If our actual nested fluent term is f(?p, table), where ?p is a variable (think e.g. an action parameter) of type "person" (a subtype of thing),
	// and table is a constant, the only possible values we will want to iterate through are [person1, person2] for the first parameter, and [table] for the second,
	// but none of the other "thing" objects.
	std::vector<const std::vector<object_id>*> possible_values;
	
	for (unsigned i = 0; i < subterms.size(); ++i) {
		const fs::Term* subterm = subterms[i];
		
		if (auto constant = dynamic_cast<const fs::Constant*>(subterm)) {
			auto v = new std::vector<object_id>(1, constant->getValue()); // Create a temporary vector with the constant value as the only value
			possible_values.push_back(v);
			_temporary_vectors.push_back(v); // store the pointer so that we can delete it later
			continue;
		} 
		
		// By default, we consider all types according to the declared signature
		// Note that in the case of bound variables, the declared type might be a subtype of that of the signature
		TypeIdx type = fs::type(*subterm);
		possible_values.push_back(&(info.getTypeObjects(type)));
	}
	
	return possible_values;
}

const term_list_iterator& term_list_iterator::operator++() {
	++(*_iterator);
	return *this;
}

bool term_list_iterator::ended() const { return _iterator->ended(); }

Gecode::IntArgs term_list_iterator::getIntArgsElement(int element) const {
	std::vector<object_id> point = arguments(); // We copy the vector to be able to add an extra element
	std::vector<int> values = fs0::values<int>(point, ObjectTable::EMPTY_TABLE);
	values.push_back(element);
	return Gecode::IntArgs(values);
}

Gecode::IntArgs term_list_iterator::getIntArgsElement() const {
	return Gecode::IntArgs(fs0::values<int>(arguments(), ObjectTable::EMPTY_TABLE));
}

const std::vector<object_id>& term_list_iterator::arguments() const { return *(*_iterator); }
	
} } // namespaces
