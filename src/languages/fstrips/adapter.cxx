
#include <languages/fstrips/adapter.hxx>
#include <constraints/builtin_constraints.hxx>
#include <problem.hxx>

namespace fs0 { namespace language { namespace fstrips {

std::vector<ScopedConstraint::cptr> Adapter::generate(const AtomicFormula& formula) {
	unsigned l1 = formula.lhs->nestedness();
	unsigned l2 = formula.rhs->nestedness();
	if (l1 == 0 && l2 == 0) return generate0(formula);
	
	throw UnimplementedFeatureException("Nested fluents... almost there - try Gecode!");
}

std::vector<ScopedConstraint::cptr> Adapter::generate0(const AtomicFormula& formula) {
	VariableIdxVector scope;
	formula.computeScope(scope);
	
	if (scope.size() > 2) throw std::runtime_error("Too high a scope for native constraints");
	
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
		return { instantiateUnaryConstraint(formula.symbol(), scope, parameters) };
	}
	
	if (lhs_const && rhs_var) { // c = X
		VariableIdxVector scope{rhs_var->getValue()};
		ObjectIdxVector parameters{lhs_const->getValue()};
		return { instantiateUnaryConstraint(formula.symbol(), scope, parameters) };
	}
	
	// Otherwise we have some complex term of the form e.g. next(d, current) != undefined  (where next is static, current is fluent.
	// We compile it into extensional form.
	assert(scope.size() == 1 || scope.size() == 2);
	
	
	
	if (scope.size() == 1) {
		return {
			new CompiledUnaryConstraint(scope, [&formula, &scope](ObjectIdx value) {
				return formula.interpret(Projections::zip(scope, {value}));
			})
		};
	}
	else {
		return {
			new CompiledBinaryConstraint(scope, [&formula, &scope](ObjectIdx x1, ObjectIdx x2) {
				return formula.interpret(Projections::zip(scope, {x1, x2}));
			})
		};
	}
}

/*
std::tuple<ObjectIdxVector, VariableIdxVector> Adapter::classify(const std::vector<Term::cptr>& terms) {
	ObjectIdxVector constants;
	VariableIdxVector variables;
	
	for (Term::cptr term:terms) {
		if (StateVariable::cptr variable = dynamic_cast<StateVariable::cptr>(term)) variables.push_back(variable->getValue());
		else if (Constant::cptr constant = dynamic_cast<Constant::cptr>(term)) constants.push_back(constant->getValue());
		else throw std::runtime_error("Unrecognized term type");
	}
	
	return std::make_tuple(constants, variables);
}*/

ScopedConstraint::cptr Adapter::instantiateUnaryConstraint(AtomicFormula::RelationSymbol symbol, const VariableIdxVector& scope, const std::vector<int>& parameters) {
	
	switch (symbol) { // EQ, NEQ, LT, LEQ, GT, GEQ
		case AtomicFormula::RelationSymbol::EQ:
			return new EQXConstraint(scope, parameters);
			break;
			
		case AtomicFormula::RelationSymbol::NEQ:
			return new NEQXConstraint(scope, parameters);
			break;
			
		default:
			// WARNING When implementing this, remember that we need to take into account what come first, whether the constant or the variable
			throw UnimplementedFeatureException("This type of relation-based constraint has not yet been implemented");
	}
}

ScopedConstraint::cptr Adapter::instantiateBinaryConstraint(AtomicFormula::RelationSymbol symbol, const VariableIdxVector& scope, const std::vector<int>& parameters) {
	
	switch (symbol) { // EQ, NEQ, LT, LEQ, GT, GEQ
		case AtomicFormula::RelationSymbol::EQ:
			return new EQConstraint(scope, parameters);
			break;
			
		case AtomicFormula::RelationSymbol::NEQ:
			return new NEQConstraint(scope, parameters);
			break;
			
		case AtomicFormula::RelationSymbol::LT:
			return new LTConstraint(scope, parameters);
			break;
		
		case AtomicFormula::RelationSymbol::LEQ:
			return new LEQConstraint(scope, parameters);
			break;
			
		default:
			throw UnimplementedFeatureException("This type of relation-based constraint has not yet been implemented");
	}
}

} } } // namespaces
