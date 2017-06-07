
#include <constraints/direct/translators/translator.hxx>
#include <languages/fstrips/terms.hxx>
#include <languages/fstrips/effects.hxx>
#include <constraints/direct/builtin.hxx>
#include <constraints/direct/compiled.hxx>
#include <problem.hxx>
#include <constraints/registry.hxx>
#include <lapkt/tools/logging.hxx>
#include <utils/projections.hxx>
#include <languages/fstrips/scopes.hxx>
#include <languages/fstrips/operations.hxx>
#include <constraints/direct/translators/effects.hxx>


namespace fs0 {


void DirectTranslator::checkSupported(const fs::Term* lhs, const fs::Term* rhs) {
	unsigned l1 = fs::nestedness(*lhs);
	unsigned l2 = fs::nestedness(*rhs);
	if (l1 > 0 || l2 > 0) throw UnimplementedFeatureException("Cannot translate nested fluents to DirectConstraints --- try Gecode instead!");
}


DirectConstraint* DirectTranslator::generate(const fs::AtomicFormula& formula) {
	if (auto relational = dynamic_cast<const fs::RelationalFormula*>(&formula)) return generate(*relational);
	
	// Else, it must be a built-in, or external condition
	auto instance = LogicalComponentRegistry::instance().instantiate_direct_constraint(formula);
	if (!instance) { // No constraint translator was registered, thus we try to extensionalize the formula
		instance = extensionalize(formula);
		if (!instance) throw std::runtime_error("No constraint translator specified for externally defined formula");
	}
	return instance;
}

DirectConstraint* DirectTranslator::generate(const fs::RelationalFormula& formula) {
	checkSupported(formula.lhs(), formula.rhs());
	
	VariableIdxVector formula_scope = fs::ScopeUtils::computeDirectScope(&formula);
	if (formula_scope.size() > 2) throw std::runtime_error("Too high a scope for direct constraints");
	
	// Here we can assume that the scope is <= 2 and there are no nested fluents
	const fs::Term* lhs = formula.lhs(), *rhs = formula.rhs(); // shortcuts
	
	auto lhs_var = dynamic_cast<const fs::StateVariable*>(lhs);
	auto rhs_var = dynamic_cast<const fs::StateVariable*>(rhs);
	auto lhs_const = dynamic_cast<const fs::Constant*>(lhs);
	auto rhs_const = dynamic_cast<const fs::Constant*>(rhs);
	
	if (lhs_const && rhs_const) { // A comparison between two constants... shouldn't get to this point
		throw std::runtime_error("Comparison between two constants");
	}
	
	if (lhs_var && rhs_var) { // X = Y
		VariableIdxVector scope{lhs_var->getValue(), rhs_var->getValue()};
		return instantiateBinaryConstraint(formula.symbol(), scope, {});
	}
	
	if (lhs_var && rhs_const) { // X = c
		VariableIdxVector scope{lhs_var->getValue()};
		std::vector<ObjectIdx> parameters{rhs_const->getValue()};
		return instantiateUnaryConstraint(formula.symbol(), scope, parameters, false);
	}
	
	if (lhs_const && rhs_var) { // c = X
		VariableIdxVector scope{rhs_var->getValue()};
		std::vector<ObjectIdx> parameters{lhs_const->getValue()};
		return instantiateUnaryConstraint(formula.symbol(), scope, parameters, true);
	}
	
	// Otherwise we have some complex term of the form e.g. next(d, current) != undefined  (where next is static, current is fluent.
	// We compile it into extensional form.
	assert(formula_scope.size() == 1 || formula_scope.size() == 2);
	return extensionalize(formula);
}

DirectConstraint* DirectTranslator::extensionalize(const fs::AtomicFormula& formula) {
	VariableIdxVector scope = fs::ScopeUtils::computeDirectScope(&formula);
	
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

std::vector<DirectConstraint*> DirectTranslator::generate(const std::vector<const fs::Formula*> formulae) {
	return generate(fs::check_all_atomic_formulas(formulae));
}

std::vector<DirectConstraint*> DirectTranslator::generate(const std::vector<const fs::AtomicFormula*> formulae) {
	std::vector<DirectConstraint*> generated;
	for (const auto formula:formulae) {
		auto translated = generate(*formula);
		LPT_DEBUG("components", "Transformed formula " << *formula << " into DirectConstraint " << *translated);
		generated.push_back(translated);
	}
	return generated;
}


const DirectEffect* DirectTranslator::generate(const fs::ActionEffect& effect) {
	checkSupported(effect.lhs(), effect.rhs());
	auto lhs_var = dynamic_cast<const fs::StateVariable*>(effect.lhs());
	if (!lhs_var) throw std::runtime_error("Direct effects accept only state variables on the LHS of an effect");
	VariableIdx affected = lhs_var->getValue();
	
	const fs::Term& rhs = *effect.rhs();
	
	VariableIdxVector rhs_scope = fs::ScopeUtils::computeDirectScope(effect.rhs());
	if (rhs_scope.size() > 2) throw std::runtime_error("Too high a scope for direct effects");
	
	auto translator = LogicalComponentRegistry::instance().getDirectEffectTranslator(rhs);
	if (translator) {
		auto translated = translator->translate(*lhs_var, rhs);
		if (translated) return translated;
	}
	
	// If there was no particular translator registered for the type of term, or the registered translator couldn't handle the term,
	// we fall back to (try to) extensionalize the constraint
	LPT_DEBUG("components", "Could not find a suitable translator for the following effect, falling back to extensionalization: " << effect);
	if (rhs_scope.size() == 1) {
		return new CompiledUnaryEffect(rhs_scope[0], affected, rhs);
	} else {
		assert(rhs_scope.size() == 2);
		return new CompiledBinaryEffect(rhs_scope, affected, rhs);
	}
}

std::vector<const DirectEffect*> DirectTranslator::generate(const std::vector<const fs::ActionEffect*>& effects) {
	std::vector<const DirectEffect*> generated;
	for (const auto effect:effects) {
		generated.push_back(generate(*effect));
	}
	return generated;
}


DirectConstraint* DirectTranslator::instantiateUnaryConstraint(fs::RelationalFormula::Symbol symbol, const VariableIdxVector& scope, const std::vector<int>& parameters, bool invert) {
	
	// Note that in some cases we might want to "invert" the relation, so that e.g. for c < X we indeed post a X > c constraint.
	switch (symbol) { // EQ, NEQ, LT, LEQ, GT, GEQ
		case fs::RelationalFormula::Symbol::EQ:
			return new EQXConstraint(scope, parameters);

		case fs::RelationalFormula::Symbol::NEQ:
			return new NEQXConstraint(scope, parameters);

		case fs::RelationalFormula::Symbol::LT:
			return invert ?  static_cast<DirectConstraint*>(new GTXConstraint(scope, parameters)) : static_cast<DirectConstraint*>(new LTXConstraint(scope, parameters));

		case fs::RelationalFormula::Symbol::LEQ:
			return invert ?  static_cast<DirectConstraint*>(new GEQXConstraint(scope, parameters)) : static_cast<DirectConstraint*>(new LEQXConstraint(scope, parameters));
			
		case fs::RelationalFormula::Symbol::GT:
			return invert ?  static_cast<DirectConstraint*>(new LTXConstraint(scope, parameters)) : static_cast<DirectConstraint*>(new GTXConstraint(scope, parameters));
			
		case fs::RelationalFormula::Symbol::GEQ:
			return invert ?  static_cast<DirectConstraint*>(new LEQXConstraint(scope, parameters)) : static_cast<DirectConstraint*>(new GEQXConstraint(scope, parameters));
		
		default:
			throw std::runtime_error("Unknown Relational formula type");
	}
}

DirectConstraint* DirectTranslator::instantiateBinaryConstraint(fs::RelationalFormula::Symbol symbol, const VariableIdxVector& scope, const std::vector<int>& parameters) {
	
	switch (symbol) { // EQ, NEQ, LT, LEQ, GT, GEQ
		case fs::RelationalFormula::Symbol::EQ:
			return new EQConstraint(scope, parameters);
			break;
			
		case fs::RelationalFormula::Symbol::NEQ:
			return new NEQConstraint(scope, parameters);
			break;
			
		case fs::RelationalFormula::Symbol::LT:
			return new LTConstraint(scope, parameters);
			break;
		
		case fs::RelationalFormula::Symbol::LEQ:
			return new LEQConstraint(scope, parameters);
			break;
			
		default:
			throw UnimplementedFeatureException("This type of relation-based constraint has not yet been implemented");
	}
}

} // namespaces
