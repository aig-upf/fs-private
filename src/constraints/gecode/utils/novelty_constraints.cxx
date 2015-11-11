

#include <constraints/gecode/utils/novelty_constraints.hxx>

namespace fs0 { namespace gecode {
	
NoveltyConstraint* NoveltyConstraint::createFromEffects(GecodeCSPVariableTranslator& translator, const fs::Formula::cptr precondition, const std::vector<fs::ActionEffect::cptr>& effects) {
	if (StrongNoveltyConstraint::applicable(effects)) {
		return new StrongNoveltyConstraint(translator, effects);
	} else {
		// Weak novelty constraints are only applicable for plain conjunctions or existentially quantified conjunctions
		return WeakNoveltyConstraint::create(translator, precondition, effects);
	}
}

} } // namespaces
