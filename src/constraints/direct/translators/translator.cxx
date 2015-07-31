
#include <constraints/direct/translators/translator.hxx>
#include <constraints/direct/builtin.hxx>
#include <constraints/direct/compiled.hxx>
#include <problem.hxx>

namespace fs0 {


DirectConstraint::cptr DirectTranslator::generate(const AtomicFormula& formula) {
	
	
	unsigned l1 = formula.lhs->nestedness();
	unsigned l2 = formula.rhs->nestedness();
	if (l1 > 0 || l2 > 0) throw UnimplementedFeatureException("Cannot translate nested fluents to DirectConstraints- try Gecode instead!");
	
	VariableIdxVector formula_scope = formula.computeScope();
	if (formula_scope.size() > 2) throw std::runtime_error("Too high a scope for native constraints");
	
	// Here we can assume that the scope is <= 2 and there are no nested fluents
	Term::cptr lhs = formula.lhs, rhs = formula.rhs; // shortcuts
	
	auto lhs_var = dynamic_cast<StateVariable::cptr>(lhs);
	auto rhs_var = dynamic_cast<StateVariable::cptr>(rhs);
	auto lhs_const = dynamic_cast<Constant::cptr>(lhs);
	auto rhs_const = dynamic_cast<Constant::cptr>(rhs);
	
	if (lhs_const && rhs_const) { // A comparison between two constants... shouldn't get to this point
		throw std::runtime_error("Comparison between two constants");
	}
	
	if (lhs_var && rhs_var) { // X = Y
		VariableIdxVector scope{lhs_var->getValue(), rhs_var->getValue()};
		return { instantiateBinaryConstraint(formula.symbol(), scope, {}) };
	}
	
	if (lhs_var && rhs_const) { // X = c
		VariableIdxVector scope{lhs_var->getValue()};
		ObjectIdxVector parameters{rhs_const->getValue()};
		return instantiateUnaryConstraint(formula.symbol(), scope, parameters);
	}
	
	if (lhs_const && rhs_var) { // c = X
		VariableIdxVector scope{rhs_var->getValue()};
		ObjectIdxVector parameters{lhs_const->getValue()};
		return instantiateUnaryConstraint(formula.symbol(), scope, parameters);
	}
	
	// Otherwise we have some complex term of the form e.g. next(d, current) != undefined  (where next is static, current is fluent.
	// We compile it into extensional form.
	assert(formula_scope.size() == 1 || formula_scope.size() == 2);
	
	
	if (formula_scope.size() == 1) {
		return new CompiledUnaryConstraint(formula_scope, [&formula, &formula_scope](ObjectIdx value) {
			return formula.interpret(Projections::zip(formula_scope, {value}));
		});
	}
	else {
		return new CompiledBinaryConstraint(formula_scope, [&formula, &formula_scope](ObjectIdx x1, ObjectIdx x2) {
			return formula.interpret(Projections::zip(formula_scope, {x1, x2}));
		});
	}
}

std::vector<DirectConstraint::cptr> DirectTranslator::generate(const std::vector<AtomicFormula::cptr> formulae) {
	std::vector<DirectConstraint::cptr> generated;
	for (const auto formula:formulae) {
		generated.push_back(generate(*formula));
	}
	return generated;
}

DirectEffect::cptr DirectTranslator::generate(const ActionEffect& effect) {
	assert(0); // TODO - TO IMPLEMENT
	return nullptr;
}
	
std::vector<DirectEffect::cptr> DirectTranslator::generate(const std::vector<ActionEffect::cptr>& effects) {
	std::vector<DirectEffect::cptr> generated;
	for (const auto effect:effects) {
		generated.push_back(generate(*effect));
	}
	return generated;
}


DirectConstraint::cptr DirectTranslator::instantiateUnaryConstraint(AtomicFormula::Symbol symbol, const VariableIdxVector& scope, const std::vector<int>& parameters) {
	
	switch (symbol) { // EQ, NEQ, LT, LEQ, GT, GEQ
		case AtomicFormula::Symbol::EQ:
			return new EQXConstraint(scope, parameters);
			break;
			
		case AtomicFormula::Symbol::NEQ:
			return new NEQXConstraint(scope, parameters);
			break;
			
		default:
			// WARNING When implementing this, remember that we need to take into account what come first, whether the constant or the variable
			throw UnimplementedFeatureException("This type of relation-based constraint has not yet been implemented");
	}
}

DirectConstraint::cptr DirectTranslator::instantiateBinaryConstraint(AtomicFormula::Symbol symbol, const VariableIdxVector& scope, const std::vector<int>& parameters) {
	
	switch (symbol) { // EQ, NEQ, LT, LEQ, GT, GEQ
		case AtomicFormula::Symbol::EQ:
			return new EQConstraint(scope, parameters);
			break;
			
		case AtomicFormula::Symbol::NEQ:
			return new NEQConstraint(scope, parameters);
			break;
			
		case AtomicFormula::Symbol::LT:
			return new LTConstraint(scope, parameters);
			break;
		
		case AtomicFormula::Symbol::LEQ:
			return new LEQConstraint(scope, parameters);
			break;
			
		default:
			throw UnimplementedFeatureException("This type of relation-based constraint has not yet been implemented");
	}
}

} // namespaces
