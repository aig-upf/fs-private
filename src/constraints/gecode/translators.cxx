
#include <problem_info.hxx>
#include <constraints/builtin_constraints.hxx>
#include <constraints/gecode/translators.hxx>
#include <constraints/gecode/translator_repository.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/csp_translator.hxx>

namespace fs0 { namespace gecode {

// MRJ: The reason for declaring here the destructors is to help with debugging
ConstraintTranslator::~ConstraintTranslator() {}

EffectTranslator::~EffectTranslator() {}

UnaryDomainBoundsConstraintTranslator::~UnaryDomainBoundsConstraintTranslator() {}

void UnaryDomainBoundsConstraintTranslator::addConstraint(gecode::SimpleCSP& csp, const gecode::GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const {
	auto* con = dynamic_cast<UnaryDomainBoundsConstraint*>(constraint);
	assert( con != nullptr);
	// TODO - Rethink, I think it is not entirely correct... constraint should be placed not on the affected variable but on the relevant variable...
	// besides, these type of cosntraints probably do not make sense in the current CSP model?
// 		Helper::addBoundsConstraintFromDomain(csp, translator, con->getAffected()); 
}

BinaryDomainBoundsConstraintTranslator::~BinaryDomainBoundsConstraintTranslator() {}

void BinaryDomainBoundsConstraintTranslator::addConstraint(gecode::SimpleCSP& csp, const gecode::GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const {
	auto* con = dynamic_cast<BinaryDomainBoundsConstraint*>(constraint);
	assert( con != nullptr);
//       Helper::addBoundsConstraintFromDomain(csp, translator, con->getAffected()); // TODO - SAME HERE
}


void UnaryRelationTranslator::addConstraint(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const {
	UnaryParametrizedScopedConstraint* c = dynamic_cast<UnaryParametrizedScopedConstraint*>(constraint);
	if (!c) throw std::runtime_error("Trying to translate wrong type of constraint");
	auto x1 = translator.resolveVariable(csp, constraint->getScope()[0], GecodeCSPTranslator::VariableType::Input);
	auto constant = c->getBinding()[0];
	Gecode::rel(csp, x1, _type, constant);
}
	
void BinaryRelationTranslator::addConstraint(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedConstraint::cptr constraint) const {
	auto x1 = translator.resolveVariable(csp, constraint->getScope()[0], GecodeCSPTranslator::VariableType::Input);
	auto x2 = translator.resolveVariable(csp, constraint->getScope()[1], GecodeCSPTranslator::VariableType::Input);
	Gecode::rel(csp, x1, _type, x2);
}


//! Perform the actual registration of all available translators (to be invoked during bootstrap)
void registerTranslators() {
	// Translators for the automatic domain-bounds constraints
	TranslatorRepository::instance().addEntry(typeid(UnaryDomainBoundsConstraint),    new UnaryDomainBoundsConstraintTranslator());
	TranslatorRepository::instance().addEntry(typeid(BinaryDomainBoundsConstraint),   new BinaryDomainBoundsConstraintTranslator());
	
	
	// Translators for built-in constraints
// 	ConstraintTranslatorRegistrar<LTConstraintTranslator> registrarUnaryDomainBounds( typeid( UnaryDomainBoundsConstraint ) );
	TranslatorRepository::instance().addEntry(typeid(LTConstraint),   new BinaryRelationTranslator(Gecode::IRT_LE)); // X < Y
	TranslatorRepository::instance().addEntry(typeid(LEQConstraint),  new BinaryRelationTranslator(Gecode::IRT_LQ)); // X <= Y
	TranslatorRepository::instance().addEntry(typeid(EQConstraint),   new BinaryRelationTranslator(Gecode::IRT_EQ)); // X = Y
	TranslatorRepository::instance().addEntry(typeid(NEQConstraint),  new BinaryRelationTranslator(Gecode::IRT_NQ)); // X != Y
	
	TranslatorRepository::instance().addEntry(typeid(NEQXConstraint), new UnaryRelationTranslator(Gecode::IRT_NQ)); // 
}

} } // namespaces
