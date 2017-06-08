
#include <algorithm>

#include <languages/fstrips/operations/binding.hxx>
#include <languages/fstrips/formulae.hxx>
#include <languages/fstrips/terms.hxx>
#include <languages/fstrips/builtin.hxx>
#include <constraints/registry.hxx>
#include <utils/binding.hxx>
#include <utils/utils.hxx>
#include <problem_info.hxx>
#include <lapkt/tools/logging.hxx>

namespace fs0 { namespace language { namespace fstrips {

const Formula* bind(const Formula& element, const Binding& binding, const ProblemInfo& info) {
	FormulaBindingVisitor visitor(binding, info);
	element.Accept(visitor);
	return visitor._result;
}

void FormulaBindingVisitor::Visit(const Formula& lhs) { throw UnimplementedFeatureException(""); }
void FormulaBindingVisitor::Visit(const Tautology& lhs) { _result = new Tautology; }
void FormulaBindingVisitor::Visit(const Contradiction& lhs) { _result = new Contradiction; }

void FormulaBindingVisitor::
Visit(const AtomicFormula& lhs) {
	// Process the subterms first
	std::vector<object_id> constant_values;
	std::vector<const Term*> processed_subterms = bind_subterms(lhs.getSubterms(), _binding, _info, constant_values);

	// Create the corresponding relational or external formula object, according to the symbol
	const AtomicFormula* processed = lhs.clone(processed_subterms);

	// Check if we can resolve the value of the formula statically
	if (constant_values.size() == lhs.getSubterms().size()) {
		auto resolved = processed->interpret(PartialAssignment()) ? static_cast<const Formula*>(new Tautology) : static_cast<const Formula*>(new Contradiction);
		delete processed;
		_result =  resolved;
	} else {
		_result = processed;
	}
}


void FormulaBindingVisitor::
Visit(const AxiomaticFormula& lhs) {
	// Process the subterms first
	std::vector<object_id> constant_values;
	std::vector<const Term*> processed_subterms = bind_subterms(lhs.getSubterms(), _binding, _info, constant_values);

	// Create the corresponding relational or external formula object, according to the symbol
	_result = lhs.clone(processed_subterms);
}


void FormulaBindingVisitor::
Visit(const Conjunction& lhs) {
	std::vector<const Formula*> conjuncts;
	std::vector<AtomConjunction::AtomT> atoms;
	for (const Formula* c:lhs.getSubformulae()) {
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
		conjuncts.push_back(processed);
		const EQAtomicFormula* cc = dynamic_cast<const EQAtomicFormula*>(processed);
		if( cc == nullptr ) continue;
		const StateVariable* lhs_ = dynamic_cast<const StateVariable*>(cc->lhs());
		if( lhs_ == nullptr ) continue;
		const Constant* rhs = dynamic_cast< const Constant*>(cc->rhs());
		if( rhs == nullptr ) continue;
		atoms.push_back(std::make_pair(lhs_->getValue(), rhs->getValue()));
	}

	if (conjuncts.empty()) {
		_result = new Tautology; // The empty conjunction is a tautology

	} else if ( conjuncts.size() == atoms.size()) { // All the subformulae are atoms
		_result = new AtomConjunction(conjuncts, atoms);

	} else {
		_result =  new Conjunction(conjuncts);
	}
}


void FormulaBindingVisitor::
Visit(const Disjunction& lhs) {
	std::vector<const Formula*> disjuncts;
	for (const Formula* c:lhs.getSubformulae()) {
		auto processed = bind(*c, _binding, _info);
		// Static checks
		if (processed->is_tautology()) { // The whole disjunction statically resolves to true
			delete processed;
			for (auto elem:disjuncts) delete elem;
			_result = new Tautology;
			return;

		} else if (processed->is_contradiction()) { // No need to add the condition, which is always false
			delete processed;
			continue;
		}
		disjuncts.push_back(processed);
	}

	if (disjuncts.empty()) {
		_result = new Contradiction; // The empty disjunction is a Contradiction

	} else {
		_result =  new Disjunction(disjuncts);
	}
}


void FormulaBindingVisitor::
Visit(const ExistentiallyQuantifiedFormula& lhs) {
	// Check that the provided binding is not binding a variable which is actually re-bound again by the current existential quantifier

	for (const BoundVariable* var:lhs.getVariables()) {
		if (_binding.binds(var->getVariableId())) {
			throw std::runtime_error("Wrong binding - Duplicated variable");
		}
	}
	// TODO Check if the binding is a complete binding and thus we can directly return the (variable-free) conjunction
	// TODO Redesign this mess
	auto bound_subformula = bind(*lhs.getSubformula(), _binding, _info);
	if (dynamic_cast<const Tautology*>(bound_subformula) || dynamic_cast<const Contradiction*>(bound_subformula)) {
		_result =  bound_subformula;
		return;
	}

	_result = new ExistentiallyQuantifiedFormula(Utils::clone(lhs.getVariables()), bound_subformula);
}

void FormulaBindingVisitor::
Visit(const UniversallyQuantifiedFormula& lhs) {
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

	_result = new UniversallyQuantifiedFormula(Utils::clone(lhs.getVariables()), bound_subformula);
}


const Term* bind(const Term& element, const Binding& binding, const ProblemInfo& info) {
	TermBindingVisitor visitor(binding, info);
	element.Accept(visitor);
	return visitor._result;
}

//! Nothing to be done for binding, simply return a clone of the element
void TermBindingVisitor::Visit(const StateVariable& lhs) { _result = lhs.clone(); }
void TermBindingVisitor::Visit(const Constant& lhs) { _result = lhs.clone(); }

void TermBindingVisitor::
Visit(const BoundVariable& lhs) {
	if (!_binding.binds(lhs.getVariableId())) {
		_result =  lhs.clone();
	} else {
		object_id value = _binding.value(lhs.getVariableId());
		_result = _info.isBoundedType(lhs.getType()) ? new IntConstant(value) : new Constant(value);
	}
}


void TermBindingVisitor::Visit(const StaticHeadedNestedTerm& lhs) { Visit(static_cast<const NestedTerm&>(lhs)); }

void TermBindingVisitor::Visit(const AdditionTerm& lhs) { Visit(static_cast<const ArithmeticTerm&>(lhs)); }
void TermBindingVisitor::Visit(const SubtractionTerm& lhs) { Visit(static_cast<const ArithmeticTerm&>(lhs)); }
void TermBindingVisitor::Visit(const MultiplicationTerm& lhs) { Visit(static_cast<const ArithmeticTerm&>(lhs)); }



void TermBindingVisitor::
Visit(const NestedTerm& lhs) {
	const auto& subterms = lhs.getSubterms();
	const auto& symbol_id = lhs.getSymbolId();

	const auto& function = _info.getSymbolData(symbol_id);

    std::vector<object_id> constant_values;
    std::vector<const Term*> st = bind_subterms(subterms, _binding, _info, constant_values);

	// If the function has unbounded arity, we cannot tell statically whether it can be resolved to a constant value or not
	if (function.hasUnboundedArity()) {
        _result = LogicalComponentRegistry::instance().instantiate_term( _info.getSymbolName(symbol_id), st);
		return;
	}


	// We process the 4 different possible cases separately:
	if (function.isStatic() && constant_values.size() == subterms.size()) { // If all subterms are constants, we can resolve the value of the term schema statically
		for (const auto ptr:st) delete ptr;
		auto value = function.getFunction()(constant_values);
		_result = _info.isBoundedType(function.getCodomainType()) ? new IntConstant(value) : new Constant(value);
	}
	else if (function.isStatic() && constant_values.size() != subterms.size() ) { // We have a statically-headed nested term
		_result = new UserDefinedStaticTerm(symbol_id, st);
	}
	else if (!function.isStatic() && constant_values.size() == subterms.size()) { // If all subterms were constant, and the symbol is fluent, we have a state variable
		VariableIdx id = _info.resolveStateVariable(symbol_id, constant_values);
// 		for (const auto ptr:st) delete ptr;
		_result =  new StateVariable(id, new FluentHeadedNestedTerm(symbol_id, st));
	}
	else {
		_result =  new FluentHeadedNestedTerm(symbol_id, st);
	}
}



void TermBindingVisitor::
Visit(const ArithmeticTerm& lhs) {
	const auto& subterms = lhs.getSubterms();

	std::vector<object_id> constant_values;
	std::vector<const Term*> st = bind_subterms(subterms, _binding, _info, constant_values);

	auto processed = lhs.create(st);

	if (constant_values.size() == subterms.size()) { // If all subterms are constants, we can resolve the value of the term schema statically
		auto value = processed->interpret(PartialAssignment(), Binding::EMPTY_BINDING);
		delete processed;
		_result = new IntConstant(value); // Arithmetic terms necessarily involve integer subterms
	}
	else {
		_result = processed;
	}
}


void TermBindingVisitor::
Visit(const UserDefinedStaticTerm& lhs) {
	const auto& subterms = lhs.getSubterms();
	const auto& symbol_id = lhs.getSymbolId();

	std::vector<object_id> constant_values;
	std::vector<const Term*> processed = bind_subterms(subterms, _binding, _info, constant_values);

	_result =  new UserDefinedStaticTerm(symbol_id, processed);

//     const auto& function = _info.getSymbolData(symbol_id);
	/*
	if (constant_values.size() == subterms.size()) { // If all subterms are constants, we can resolve the value of the term schema statically
		for (const auto ptr:processed) delete ptr;

		auto value = function.getFunction()(constant_values);
		_result = _info.isBoundedType(function.getCodomainType()) ? new IntConstant(value) : new Constant(value);

	} else {
		// Otherwise we simply return a user-defined static term with the processed/bound subterms
		_result =  new UserDefinedStaticTerm(symbol_id, processed);
	}
	*/
}

void TermBindingVisitor::
Visit(const AxiomaticTermWrapper& lhs) {
	std::vector<object_id> constant_values;
	std::vector<const Term*> processed = bind_subterms(lhs.getSubterms(), _binding, _info, constant_values);

	// we simply return a new axiomaticTerm with the processed/bound subterms
	_result =  new AxiomaticTermWrapper(lhs.getAxiom(), lhs.getSymbolId(), processed);
}


void TermBindingVisitor::
Visit(const AxiomaticTerm& lhs) {
	const auto& subterms = lhs.getSubterms();
	std::vector<object_id> constant_values;
	std::vector<const Term*> processed = bind_subterms(subterms, _binding, _info, constant_values);

	// We simply return a user-defined static term with the processed/bound subterms
	_result = lhs.clone(processed);
}


void TermBindingVisitor::
Visit(const FluentHeadedNestedTerm& lhs) {
	const auto& subterms = lhs.getSubterms();
	const auto& symbol_id = lhs.getSymbolId();

	std::vector<object_id> constant_values;
	std::vector<const Term*> processed = bind_subterms(subterms, _binding, _info, constant_values);
	
    LPT_DEBUG( "binding", "Binding (FluentHeadedNestedTerm): " << lhs );
	if (constant_values.size() == subterms.size()) { // If all subterms were constant, and the symbol is fluent, we have a state variable
		VariableIdx id = _info.resolveStateVariable(symbol_id, constant_values);
		_result =  new StateVariable(id, new FluentHeadedNestedTerm(symbol_id, processed));

	} else {
		_result = new FluentHeadedNestedTerm(symbol_id, processed);
	}
}



std::vector<const Term*>
bind_subterms(const std::vector<const Term*>& subterms, const Binding& binding, const ProblemInfo& info, std::vector<object_id>& constants) {
	assert(constants.empty());
	std::vector<const Term*> result;
	for (auto unprocessed:subterms) {
		auto processed = bind(*unprocessed, binding, info);
		result.push_back(processed);

		if (const Constant* constant = dynamic_cast<const Constant*>(processed)) {
			constants.push_back(constant->getValue());
		}
	}
	return result;
}

} } } // namespaces
