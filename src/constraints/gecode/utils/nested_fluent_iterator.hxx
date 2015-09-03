

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
	fs::FluentHeadedNestedTerm::cptr _term;
	
	// We store the temporary vectors to delete them afterwards
	std::vector<const std::vector<ObjectIdx>*> _temporary_vectors;
	
	//! The iterator of the actual cartesian product of values
	utils::cartesian_iterator _iterator;
	
	//! The current index in the iteration
	unsigned _index;


public:
	nested_fluent_iterator(fs::FluentHeadedNestedTerm::cptr term) :
		_term(term),
		_iterator(compute_possible_values(term)),
		_index(0)
	{}
	
	~nested_fluent_iterator() {
		for (auto vector:_temporary_vectors) delete vector;
	}
	
	std::vector<const std::vector<ObjectIdx>*> compute_possible_values(fs::FluentHeadedNestedTerm::cptr term) {
		const ProblemInfo& info = Problem::getInfo();
		const Signature& signature = info.getFunctionData(term->getSymbolId()).getSignature();
		const std::vector<fs::Term::cptr>& subterms = term->getSubterms();
		assert(subterms.size() == signature.size());
		assert(signature.size() > 0); // Cannot be 0, or we'd have instead a StateVariable term
		
		auto objects = info.getTypeObjects(signature[0]);
		std::vector<const std::vector<ObjectIdx>*> possible_values = info.getSignatureValues(signature);
		
		// If a subterm is a constant, there is no need to iterate through all possible values for that type
		for (unsigned i = 0; i < subterms.size(); ++i) {
			if (auto constant = dynamic_cast<fs::Constant::cptr>(term)) {
				auto v = new std::vector<ObjectIdx>(1, constant->getValue()); // Create a temporary vector with the constant value as the only value
				possible_values[i] = v;
				_temporary_vectors.push_back(v);
			}
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
	
	Gecode::IntArgs getIntArgsElement() const {
		std::vector<ObjectIdx> point = *_iterator; // We copy the vector to be able to add an extra element
		point.push_back(getIndex());
		return Gecode::IntArgs(point);

	}

};

} } // namespaces
