
#include <constraints/direct/bound_constraint.hxx>
#include <constraints/direct/compiled.hxx>
#include <problem.hxx>
#include <problem_info.hxx>

namespace fs0 {


UnaryDomainBoundsConstraint::UnaryDomainBoundsConstraint(const UnaryDirectEffect* effect, const ProblemInfo& problemInfo) :
	UnaryDirectConstraint(effect->getScope(), {}), _problemInfo(problemInfo), _effect(effect)
{}

// The domain constraint is satisfied iff the application of the corresponding effect keeps the affected
// value within the domain bounds.
bool UnaryDomainBoundsConstraint::isSatisfied(ObjectIdx o) const {
		return _effect->applicable(o) && _problemInfo.checkValueIsValid(_effect->apply(o));
}


BinaryDomainBoundsConstraint::BinaryDomainBoundsConstraint(const BinaryDirectEffect* effect, const ProblemInfo& problemInfo) :
	BinaryDirectConstraint(effect->getScope(), {}), _problemInfo(problemInfo), _effect(effect)
{}

bool BinaryDomainBoundsConstraint::isSatisfied(ObjectIdx o1, ObjectIdx o2) const {
		return _effect->applicable(o1, o2) &&  _problemInfo.checkValueIsValid(_effect->apply(o1, o2));
}

void BoundsConstraintsGenerator::generate(const GroundAction& action, const std::vector<DirectEffect::cptr>& effects, std::vector<DirectConstraint::cptr>& constraints) {
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	for (const DirectEffect::cptr effect:effects) {
		VariableIdx affected = effect->getAffected();
		if (!info.isBoundedVariable(affected)) continue;

		// We process the creation of the bound-constraint differently for each arity
		if (dynamic_cast<const ZeroaryDirectEffect*>(effect)) {
			if (!info.checkValueIsValid(effect->apply())) {
				std::stringstream buffer;
				buffer << "Error: 0-ary effect '" << *effect << "' of action " << action << " produces out-of-bounds variable values";
				throw std::runtime_error(buffer.str());
			}
		} else if (const auto* casted = dynamic_cast<const UnaryDirectEffect*>(effect)) {
			constraints.push_back(new UnaryDomainBoundsConstraint(casted, info));
		} else if (const auto* casted = dynamic_cast<const BinaryDirectEffect*>(effect)) {
			constraints.push_back(new BinaryDomainBoundsConstraint(casted, info));
		} 
		else throw std::runtime_error("Shouldn't be processing an action having effects with arity > 2 here - try the GecodeActionManager instead");
	}
}


} // namespaces
