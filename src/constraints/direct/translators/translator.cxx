
#include <constraints/direct/translators/translator.hxx>
#include <constraints/direct/builtin.hxx>
#include <constraints/direct/compiled.hxx>
#include <problem.hxx>
#include <constraints/registry.hxx>
#include <utils/logging.hxx>
#include <languages/fstrips/scopes.hxx>

namespace fs0 {


void DirectTranslator::checkSupported(const fs::Term::cptr lhs, const fs::Term::cptr rhs) {
	unsigned l1 = lhs->nestedness();
	unsigned l2 = rhs->nestedness();
	if (l1 > 0 || l2 > 0) throw UnimplementedFeatureException("Cannot translate nested fluents to DirectConstraints --- try Gecode instead!");
}


DirectConstraint::cptr DirectTranslator::generate(const AtomicFormula& formula) {
	if (auto relational = dynamic_cast<fs::RelationalFormula::cptr>(&formula)) return generate(*relational);
	
	// Else, it must be a built-in, or external condition
	auto instance = LogicalComponentRegistry::instance().instantiate_direct_constraint(formula);
	if (!instance) { // No constraint translator was registered, thus we try to extensionalize the formula
		instance = extensionalize(formula);
		if (!instance) throw std::runtime_error("No constraint translator specified for externally defined formula");
	}
	return instance;
}

DirectConstraint::cptr DirectTranslator::generate(const RelationalFormula& formula) {
	checkSupported(formula.lhs(), formula.rhs());
	
	VariableIdxVector formula_scope = ScopeUtils::computeDirectScope(&formula);
	if (formula_scope.size() > 2) throw std::runtime_error("Too high a scope for direct constraints");
	
	// Here we can assume that the scope is <= 2 and there are no nested fluents
	Term::cptr lhs = formula.lhs(), rhs = formula.rhs(); // shortcuts
	
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
		return instantiateUnaryConstraint(formula.symbol(), scope, parameters, false);
	}
	
	if (lhs_const && rhs_var) { // c = X
		VariableIdxVector scope{rhs_var->getValue()};
		ObjectIdxVector parameters{lhs_const->getValue()};
		return instantiateUnaryConstraint(formula.symbol(), scope, parameters, true);
	}
	
	// Otherwise we have some complex term of the form e.g. next(d, current) != undefined  (where next is static, current is fluent.
	// We compile it into extensional form.
	assert(formula_scope.size() == 1 || formula_scope.size() == 2);
	return extensionalize(formula);
}

DirectConstraint::cptr DirectTranslator::extensionalize(const AtomicFormula& formula) {
	VariableIdxVector scope = ScopeUtils::computeDirectScope(&formula);
	
	if (scope.size() == 1) {
		return new CompiledUnaryConstraint(scope, [&formula, &scope](ObjectIdx value) {
			return formula.interpret(Projections::zip(scope, {value}));
		});
	}
	else if (scope.size() == 2) {
		return new CompiledBinaryConstraint(scope, [&formula, &scope](ObjectIdx x1, ObjectIdx x2) {
			return formula.interpret(Projections::zip(scope, {x1, x2}));
		});
	}
	else return nullptr;
}

std::vector<DirectConstraint::cptr> DirectTranslator::generate(const std::vector<AtomicFormula::cptr> formulae) {
	std::vector<DirectConstraint::cptr> generated;
	for (const auto formula:formulae) {
		generated.push_back(generate(*formula));
	}
	return generated;
}

DirectEffect::cptr DirectTranslator::generate(const ActionEffect& effect) {
	checkSupported(effect.lhs(), effect.rhs());
	auto lhs_var = dynamic_cast<StateVariable::cptr>(effect.lhs());
	if (!lhs_var) throw std::runtime_error("Direct effects accept only state variables on the LHS of an effect");
	VariableIdx affected = lhs_var->getValue();
	
	const Term& rhs = *effect.rhs();
	
	VariableIdxVector rhs_scope = ScopeUtils::computeDirectScope(effect.rhs());
	if (rhs_scope.size() > 2) throw std::runtime_error("Too high a scope for direct effects");
	
	auto translator = LogicalComponentRegistry::instance().getDirectEffectTranslator(rhs);
	if (translator) {
		auto translated = translator->translate(*lhs_var, rhs);
		if (translated) return translated;
	}
	
	// If there was no particular translator registered for the type of term, or the registered translator couldn't handle the term,
	// we fall back to (try to) extensionalize the constraint
	FDEBUG("components", "Could not find a suitable translator for the following effect, falling back to extensionalization: " << effect);
	if (rhs_scope.size() == 1) {
		return new CompiledUnaryEffect(rhs_scope[0], affected, rhs);
	} else {
		assert(rhs_scope.size() == 2);
		return new CompiledBinaryEffect(rhs_scope, affected, rhs);
	}
}

std::vector<DirectEffect::cptr> DirectTranslator::generate(const std::vector<ActionEffect::cptr>& effects) {
	std::vector<DirectEffect::cptr> generated;
	for (const auto effect:effects) {
		generated.push_back(generate(*effect));
	}
	return generated;
}


DirectConstraint::cptr DirectTranslator::instantiateUnaryConstraint(RelationalFormula::Symbol symbol, const VariableIdxVector& scope, const std::vector<int>& parameters, bool invert) {
	
	// Note that in some cases we might want to "invert" the relation, so that e.g. for c < X we indeed post a X > c constraint.
	switch (symbol) { // EQ, NEQ, LT, LEQ, GT, GEQ
		case RelationalFormula::Symbol::EQ:
			return new EQXConstraint(scope, parameters);

		case RelationalFormula::Symbol::NEQ:
			return new NEQXConstraint(scope, parameters);

		case RelationalFormula::Symbol::LT:
			return invert ?  static_cast<DirectConstraint::cptr>(new GTXConstraint(scope, parameters)) : static_cast<DirectConstraint::cptr>(new LTXConstraint(scope, parameters));

		case RelationalFormula::Symbol::LEQ:
			return invert ?  static_cast<DirectConstraint::cptr>(new GEQXConstraint(scope, parameters)) : static_cast<DirectConstraint::cptr>(new LEQXConstraint(scope, parameters));
			
		case RelationalFormula::Symbol::GT:
			return invert ?  static_cast<DirectConstraint::cptr>(new LTXConstraint(scope, parameters)) : static_cast<DirectConstraint::cptr>(new GTXConstraint(scope, parameters));
			
		case RelationalFormula::Symbol::GEQ:
			return invert ?  static_cast<DirectConstraint::cptr>(new LEQXConstraint(scope, parameters)) : static_cast<DirectConstraint::cptr>(new GEQXConstraint(scope, parameters));
		
		default:
			assert(0);
	}
}

DirectConstraint::cptr DirectTranslator::instantiateBinaryConstraint(RelationalFormula::Symbol symbol, const VariableIdxVector& scope, const std::vector<int>& parameters) {
	
	switch (symbol) { // EQ, NEQ, LT, LEQ, GT, GEQ
		case RelationalFormula::Symbol::EQ:
			return new EQConstraint(scope, parameters);
			break;
			
		case RelationalFormula::Symbol::NEQ:
			return new NEQConstraint(scope, parameters);
			break;
			
		case RelationalFormula::Symbol::LT:
			return new LTConstraint(scope, parameters);
			break;
		
		case RelationalFormula::Symbol::LEQ:
			return new LEQConstraint(scope, parameters);
			break;
			
		default:
			throw UnimplementedFeatureException("This type of relation-based constraint has not yet been implemented");
	}
}

} // namespaces
