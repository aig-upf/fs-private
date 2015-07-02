
#include <constraints/gecode/constraint_translation.hxx>
#include <constraints/gecode/expr_translator_repository.hxx>
#include <constraints/gecode/auto_bounds_constraints.hxx>

namespace fs0 { namespace gecode {

void registerTranslators() {
	ConstraintTranslatorRegistrar< UnaryDomainBoundsConstraintTranslator > registrarUnaryDomainBounds( typeid( UnaryDomainBoundsConstraint ) );
	ConstraintTranslatorRegistrar< BinaryDomainBoundsConstraintTranslator > registrarBinaryDomainBounds( typeid( BinaryDomainBoundsConstraint ) );
}

} } // namespaces
