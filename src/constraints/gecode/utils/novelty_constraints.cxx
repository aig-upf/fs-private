

#include <constraints/gecode/utils/novelty_constraints.hxx>

namespace fs0 { namespace gecode {
	
NoveltyConstraint* NoveltyConstraint::createFromEffects(GecodeCSPVariableTranslator& translator, const std::vector<fs::AtomicFormula::cptr>& conditions, const std::vector<fs::ActionEffect::cptr>& effects) {
	if (StrongNoveltyConstraint::applicable(effects)) {
		return new StrongNoveltyConstraint(translator, effects);
	} else return WeakNoveltyConstraint::create(translator, conditions, effects);
}

} } // namespaces
