
#include <problem_info.hxx>
#include <constraints/all.hxx>
#include <constraints/builtin_effects.hxx>
#include <constraints/gecode/translators.hxx>
#include <constraints/gecode/translator_repository.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/csp_translator.hxx>

namespace fs0 { namespace gecode {

// MRJ: The reason for declaring here the destructors is to help with debugging
ConstraintTranslator::~ConstraintTranslator() {}

EffectTranslator::~EffectTranslator() {}


// We compile the unary constraint to obtain the satisfying values and turn this into a Gecode extensional constraint
void ExtensionalUnaryConstraintTranslator::registerConstraints(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const {
	auto* unaryConstraint = dynamic_cast<UnaryParametrizedScopedConstraint*>(constraint);
	if (unaryConstraint == nullptr) throw std::runtime_error("Unary Extensional constraints are not applicable to non-unary constraints");
	
	const auto& scope = constraint->getScope();
	auto variable = translator.resolveVariable(csp, scope[0], CSPVariableType::Input);
	auto values = CompiledUnaryConstraint::compile(*unaryConstraint);
	
	Gecode::TupleSet tuples;
	for (auto& value:values) {
		tuples.add( Gecode::IntArgs(1, value)); // A tuple with a single value
	}
	tuples.finalize();
	Gecode::extensional(csp, Gecode::IntVarArgs() << variable, tuples);
}

// We compile the unary constraint to obtain the satisfying values and turn this into a Gecode extensional constraint
void ExtensionalBinaryConstraintTranslator::registerConstraints(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const {
	auto* binaryConstraint = dynamic_cast<BinaryParametrizedScopedConstraint*>(constraint);
	if (binaryConstraint == nullptr) throw std::runtime_error("Binary Extensional constraints are not applicable to non-binary constraints");
	
	Gecode::IntVarArgs variables = translator.resolveScope(csp, constraint->getScope(), CSPVariableType::Input);

	// Note that we invoke and obtain the compilation for the first variable (value 0 of second parameter). Order matters.
	CompiledBinaryConstraint::TupleExtension values = CompiledBinaryConstraint::compile(*binaryConstraint);
	
	Gecode::TupleSet tuples;
	for (const auto& value:values) {
		tuples.add( Gecode::IntArgs(2, std::get<0>(value), std::get<1>(value)));
	}
	tuples.finalize();
	Gecode::extensional(csp, variables, tuples);
}



UnaryDomainBoundsConstraintTranslator::~UnaryDomainBoundsConstraintTranslator() {}

void UnaryDomainBoundsConstraintTranslator::registerConstraints(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const {
	auto* con = dynamic_cast<UnaryDomainBoundsConstraint*>(constraint);
	assert( con != nullptr);
	// TODO - Rethink, I think it is not entirely correct... constraint should be placed not on the affected variable but on the relevant variable...
	// besides, these type of cosntraints probably do not make sense in the current CSP model?
// 		Helper::addBoundsConstraintFromDomain(csp, translator, con->getAffected()); 
}

BinaryDomainBoundsConstraintTranslator::~BinaryDomainBoundsConstraintTranslator() {}

void BinaryDomainBoundsConstraintTranslator::registerConstraints(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const {
	auto* con = dynamic_cast<BinaryDomainBoundsConstraint*>(constraint);
	assert( con != nullptr);
//       Helper::addBoundsConstraintFromDomain(csp, translator, con->getAffected()); // TODO - SAME HERE
}


void UnaryRelationTranslator::registerConstraints(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const {
	UnaryParametrizedScopedConstraint* c = dynamic_cast<UnaryParametrizedScopedConstraint*>(constraint);
	if (!c) throw std::runtime_error("Trying to translate wrong type of constraint");
	auto x1 = translator.resolveVariable(csp, constraint->getScope()[0], CSPVariableType::Input);
	auto constant = c->getBinding()[0];
	Gecode::rel(csp, x1, _type, constant);
}
	
void BinaryRelationTranslator::registerConstraints(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const {
	Gecode::IntVarArgs variables = translator.resolveScope(csp, constraint->getScope(), CSPVariableType::Input);
	Gecode::rel(csp, variables[0], _type, variables[1]);
}


void SumConstraintTranslator::registerConstraints(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const {
	const VariableIdxVector& scope = constraint->getScope();
	
	Gecode::IntVarArgs variables = translator.resolveScope(csp, scope, CSPVariableType::Input);
	std::vector<int> v_coefficients(scope.size(), 1);
	v_coefficients[scope.size() - 1] = -1; // Last coefficient is a -1, since the last variable of the scope is the element of the sum
	Gecode::IntArgs coefficients(v_coefficients);
	
	Gecode::linear(csp, coefficients, variables, Gecode::IRT_EQ, 0);
}

void AlldiffConstraintTranslator::registerConstraints(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const {
	Gecode::IntVarArgs variables = translator.resolveScope(csp, constraint->getScope(), CSPVariableType::Input);
	Gecode::distinct(csp, variables);
}


void ExtensionalUnaryEffectTranslator::registerConstraints(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedEffect::cptr effect) const {
	auto* unaryEffect = dynamic_cast<const UnaryScopedEffect*>(effect);
	if (unaryEffect == nullptr) throw std::runtime_error("Unary Extensional effects are not applicable to non-unary effects");
	
	// Order is relevant: first the input variable, then the output
	Gecode::IntVarArgs variables;
	variables << translator.resolveVariable(csp, effect->getScope()[0], CSPVariableType::Input);
	variables << translator.resolveVariable(csp, effect->getAffected(), CSPVariableType::Output);
	auto map = CompiledUnaryEffect::compile(*unaryEffect, Problem::getCurrentProblem()->getProblemInfo());
	
	Gecode::TupleSet tuples;
	for (auto& value:map) {
		tuples.add(Gecode::IntArgs(2, value.first, value.second));
	}
	tuples.finalize();
	Gecode::extensional(csp, variables, tuples);
}

void ValueAssignmentEffectTranslator::registerConstraints(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedEffect::cptr effect) const {
	auto y = translator.resolveVariable(csp, effect->getAffected(), CSPVariableType::Output);
	Gecode::rel(csp, y, Gecode::IRT_EQ, effect->getBinding()[0]);
}

void VariableAssignmentEffectTranslator::registerConstraints(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedEffect::cptr effect) const {
	auto y = translator.resolveVariable(csp, effect->getAffected(), CSPVariableType::Output);
	auto x = translator.resolveVariable(csp, effect->getScope()[0], CSPVariableType::Input);
	Gecode::rel(csp, y, Gecode::IRT_EQ, x);
}

void AdditiveUnaryEffectTranslator::registerConstraints(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedEffect::cptr effect) const {
	auto y_var = translator.resolveVariable(csp, effect->getAffected(), CSPVariableType::Output);
	auto x_var = translator.resolveVariable(csp, effect->getScope()[0], CSPVariableType::Input);
	Gecode::IntArgs     coeffs(2);
	Gecode::IntVarArgs  vars(2);
	coeffs[0] = 1;
	coeffs[1] = -1;
	vars[0] = y_var;
	vars[1] = x_var;
	Gecode::linear(csp, coeffs, vars, Gecode::IRT_EQ, effect->getBinding()[0]);
}


//! Perform the actual registration of all available translators (to be invoked during bootstrap)
void registerTranslators() {
	
	// Generic translators for unary and binary constraints that simply extensionalize the constraint.
	TranslatorRepository::instance().addEntry(typeid(UnaryParametrizedScopedConstraint),    new ExtensionalUnaryConstraintTranslator());
	TranslatorRepository::instance().addEntry(typeid(BinaryParametrizedScopedConstraint),   new ExtensionalBinaryConstraintTranslator());
	TranslatorRepository::instance().addEntry(typeid(UnaryScopedEffect),                    new ExtensionalUnaryEffectTranslator());
	
	
	// Translators for the automatic domain-bounds constraints
	TranslatorRepository::instance().addEntry(typeid(UnaryDomainBoundsConstraint),    new UnaryDomainBoundsConstraintTranslator());
	TranslatorRepository::instance().addEntry(typeid(BinaryDomainBoundsConstraint),   new BinaryDomainBoundsConstraintTranslator());
	
	// Translators for built-in constraints
	// Unary:
	TranslatorRepository::instance().addEntry(typeid(EQXConstraint),  new UnaryRelationTranslator(Gecode::IRT_EQ)); // X == c
	TranslatorRepository::instance().addEntry(typeid(NEQXConstraint), new UnaryRelationTranslator(Gecode::IRT_NQ)); // X != c
	
	// Binary:
	TranslatorRepository::instance().addEntry(typeid(LTConstraint),   new BinaryRelationTranslator(Gecode::IRT_LE)); // X < Y
	TranslatorRepository::instance().addEntry(typeid(LEQConstraint),  new BinaryRelationTranslator(Gecode::IRT_LQ)); // X <= Y
	TranslatorRepository::instance().addEntry(typeid(EQConstraint),   new BinaryRelationTranslator(Gecode::IRT_EQ)); // X = Y
	TranslatorRepository::instance().addEntry(typeid(NEQConstraint),  new BinaryRelationTranslator(Gecode::IRT_NQ)); // X != Y
	
	// Translators for built-in effects
	TranslatorRepository::instance().addEntry(typeid(ValueAssignmentEffect),    new ValueAssignmentEffectTranslator()); // Y := c
	TranslatorRepository::instance().addEntry(typeid(VariableAssignmentEffect), new VariableAssignmentEffectTranslator()); // Y := X
	TranslatorRepository::instance().addEntry(typeid(AdditiveUnaryEffect),      new AdditiveUnaryEffectTranslator());   // Y := X + c
	
	
	// Translators for some global constraints
	TranslatorRepository::instance().addEntry(typeid(ScopedAlldiffConstraint),  new AlldiffConstraintTranslator());  // alldiff(X1, X2, ..., Xn)
	TranslatorRepository::instance().addEntry(typeid(ScopedSumConstraint),      new SumConstraintTranslator());      // Y = X1 + X2 + X3 + ... + Xn
	
}

} } // namespaces
