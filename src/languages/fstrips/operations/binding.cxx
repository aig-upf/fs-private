
#include <algorithm>

#include <languages/fstrips/operations/binding.hxx>
#include <languages/fstrips/terms.hxx>
#include <utils/binding.hxx>

namespace fs0 { namespace language { namespace fstrips {

const Formula* bind(const Formula& element, const Binding& binding, const ProblemInfo& info) {
	BindingVisitor visitor(binding, info);
	element.Accept(visitor);
	return visitor._result;
}



void BindingVisitor::
Visit(const AtomicFormula& lhs) {
	// Process the subterms first
	std::vector<ObjectIdx> constant_values;
	std::vector<const Term*> processed_subterms = NestedTerm::bind_subterms(lhs.getSubterms(), _binding, _info, constant_values);

	// Create the corresponding relational or external formula object, according to the symbol
	const AtomicFormula* processed = lhs.clone(processed_subterms);

	// Check if we can resolve the value of the formula statically
	if (constant_values.size() == lhs.getSubterms().size()) {
		auto resolved = processed->interpret({}) ? static_cast<const Formula*>(new Tautology) : static_cast<const Formula*>(new Contradiction);
		delete processed;
		_result =  resolved;
		return;
	}

	_result = processed;	
}


void BindingVisitor::
Visit(const AxiomaticFormula& lhs) {
	// Process the subterms first
	std::vector<ObjectIdx> constant_values;
	std::vector<const Term*> processed_subterms = NestedTerm::bind_subterms(lhs.getSubterms(), _binding, _info, constant_values);

	// Create the corresponding relational or external formula object, according to the symbol
	_result = lhs.clone(processed_subterms);
}


void BindingVisitor::
Visit(const Conjunction& lhs) {
	std::vector<const AtomicFormula*> conjuncts;
	std::vector<AtomConjunction::AtomT> atoms;
	for (const AtomicFormula* c:lhs.getConjuncts()) {
		auto processed = bind(*c, _binding, _info);
		// Static checks
		if (processed->is_tautology()) { // No need to add the condition, which is always true
			delete processed;
			continue;
		} else if (processed->is_contradiction()) { // The whole conjunction statically resolves to false
			delete processed;
			for (auto elem:conjuncts) delete elem;
			_result = new Contradiction;
			return;
		}
		auto processed_atomic = dynamic_cast<const AtomicFormula*>(processed);
		assert(processed_atomic);
		conjuncts.push_back(processed_atomic);
		const EQAtomicFormula* cc = dynamic_cast<const EQAtomicFormula*>(processed_atomic);
		if( cc == nullptr ) continue;
		const StateVariable* lhs = dynamic_cast<const StateVariable*>(cc->lhs());
		if( lhs == nullptr ) continue;
		const Constant* rhs = dynamic_cast< const Constant*>(cc->rhs());
		if( rhs == nullptr ) continue;
		atoms.push_back(std::make_pair(lhs->getValue(), rhs->getValue()));
	}

	if (conjuncts.empty()) {
		_result = new Tautology; // The empty conjunction is a tautology
		return;
	}

	if ( conjuncts.size() == atoms.size()) { // All the subformulae are atoms
		_result = new AtomConjunction(conjuncts, atoms);
		return;
	}

	_result =  new Conjunction(conjuncts);
}


void BindingVisitor::
Visit(const ExistentiallyQuantifiedFormula& lhs) {
	// Check that the provided binding is not binding a variable which is actually re-bound again by the current existential quantifier
	for (const BoundVariable* var:lhs.getVariables()) {
		if (_binding.binds(var->getVariableId())) throw std::runtime_error("Wrong binding - Duplicated variable");
	}
	// TODO Check if the binding is a complete binding and thus we can directly return the (variable-free) conjunction
	// TODO Redesign this mess
	auto bound_subformula = bind(*lhs.getSubformula(), _binding, _info);
	if (dynamic_cast<const Tautology*>(bound_subformula) || dynamic_cast<const Contradiction*>(bound_subformula)) {
		_result =  bound_subformula;
		return;
	}

	auto bound_conjunction = dynamic_cast<const Conjunction*>(bound_subformula);
	assert(bound_conjunction);

	_result = new ExistentiallyQuantifiedFormula(lhs.getVariables(), bound_conjunction);
}


} } } // namespaces
