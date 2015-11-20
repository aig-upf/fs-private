

#pragma once

#include <languages/fstrips/terms.hxx>
#include <problem.hxx>
#include <utils/cartesian_iterator.hxx>
#include <gecode/int.hh>

namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

//! A nested_fluent_iterator allows us to iterate through all the state variables that can be derived from a given nested fluent term.
class nested_fluent_iterator {
protected:
	//! The nested fluent
	fs::NestedTerm::cptr _term;
	
	// We store the temporary vectors to delete them afterwards
	std::vector<const std::vector<ObjectIdx>*> _temporary_vectors;
	
	//! The iterator of the actual cartesian product of values
	utils::cartesian_iterator _iterator;
	
	//! The current index in the iteration
	unsigned _index;


public:
	nested_fluent_iterator(fs::NestedTerm::cptr term) :
		_term(term),
		_iterator(compute_possible_values(term)),
		_index(0)
	{}
	
	~nested_fluent_iterator() {
		for (auto vector:_temporary_vectors) delete vector;
	}
	
protected:
	std::vector<const std::vector<ObjectIdx>*> compute_possible_values(fs::NestedTerm::cptr term) {
		const ProblemInfo& info = Problem::getInfo();
		const Signature& signature = info.getFunctionData(term->getSymbolId()).getSignature();
		const std::vector<fs::Term::cptr>& subterms = term->getSubterms();
		assert(subterms.size() == signature.size());
		assert(signature.size() > 0); // Cannot be 0, or we'd have instead a StateVariable term
		
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
		std::vector<const std::vector<ObjectIdx>*> possible_values;
		
		for (unsigned i = 0; i < signature.size(); ++i) {
			fs::Term::cptr subterm = subterms[i];
			TypeIdx type = signature[i];
			
			if (auto constant = dynamic_cast<fs::Constant::cptr>(subterm)) {
				auto v = new std::vector<ObjectIdx>(1, constant->getValue()); // Create a temporary vector with the constant value as the only value
				possible_values.push_back(v);
				_temporary_vectors.push_back(v); // store the pointer so that we can delete it later
				continue;
			} 
			
			// By default, consider all types according to the signature, but in the case of bound variables, trust the type of the bound variable,
			// which might a subtype of that of the signature
			if (auto variable = dynamic_cast<fs::BoundVariable::cptr>(subterm)) {
				type = variable->getType();
			}
			possible_values.push_back(&(info.getTypeObjects(type)));
		}
		
		return possible_values;
	}

public:
	const nested_fluent_iterator& operator++() {
		++_iterator;
		++_index;
		return *this;
	}
	const nested_fluent_iterator operator++(int) { nested_fluent_iterator tmp(*this); operator++(); return tmp; }
	
	bool ended() const { return _iterator.ended(); }
	
	unsigned getIndex() const { return _index; }
	
	VariableIdx getDerivedStateVariable() const {
		const ProblemInfo& info = Problem::getInfo();
		const std::vector<ObjectIdx>& point = *_iterator;
		return info.resolveStateVariable(_term->getSymbolId(), point);
	}
	
	Gecode::IntArgs getIndexedIntArgsElement() const {
		return getIntArgsElement(getIndex());
	}
	
	Gecode::IntArgs getIntArgsElement(int element) const {
		std::vector<ObjectIdx> point = arguments(); // We copy the vector to be able to add an extra element
		point.push_back(element);
		return Gecode::IntArgs(point);
	}	
	
	const std::vector<ObjectIdx>& arguments() const { return *_iterator; }

};

} } // namespaces
