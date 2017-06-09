
#include <constraints/direct/bound_constraint.hxx>
#include <constraints/direct/compiled.hxx>
#include <problem_info.hxx>
#include <utils/printers/vector.hxx>
#include <utils/printers/helper.hxx>
#include <actions/actions.hxx>

namespace fs0 {


UnaryDomainBoundsConstraint::UnaryDomainBoundsConstraint(const UnaryDirectEffect* effect, const ProblemInfo& problemInfo) :
	UnaryDirectConstraint(effect->getScope(), {}), _problemInfo(problemInfo), _effect(effect)
{}

// The domain constraint is satisfied iff the application of the corresponding effect keeps the affected
// value within the domain bounds.
bool UnaryDomainBoundsConstraint::isSatisfied(const object_id& o) const {
		return _effect->applicable(o) && _problemInfo.checkValueIsValid(_effect->apply(o));
}

std::ostream& UnaryDomainBoundsConstraint::print(std::ostream& os) const {
	const ProblemInfo& info = ProblemInfo::getInstance();
	os << "UnaryDomainBoundsConstraint[" << info.getVariableName(_scope[0]) << "]";
	return os;
}

DirectConstraint* UnaryDomainBoundsConstraint::compile(const ProblemInfo& problemInfo) const {
	return new CompiledUnaryConstraint(*this);
}
	


BinaryDomainBoundsConstraint::BinaryDomainBoundsConstraint(const BinaryDirectEffect* effect, const ProblemInfo& problemInfo) :
	BinaryDirectConstraint(effect->getScope(), {}), _problemInfo(problemInfo), _effect(effect)
{}

bool BinaryDomainBoundsConstraint::isSatisfied(const object_id& o1, const object_id& o2) const {
		return _effect->applicable(o1, o2) &&  _problemInfo.checkValueIsValid(_effect->apply(o1, o2));
}

DirectConstraint* BinaryDomainBoundsConstraint::compile(const ProblemInfo& problemInfo) const {
	return new CompiledBinaryConstraint(*this, problemInfo);
}

void BoundsConstraintsGenerator::generate(const GroundAction& action, const std::vector<const DirectEffect*>& effects, std::vector<DirectConstraint*>& constraints) {
	const ProblemInfo& info = ProblemInfo::getInstance();
	for (const DirectEffect* effect:effects) {
		VariableIdx affected = effect->getAffected();
		if (!info.isBoundedVariable(affected)) continue;

		// We process the creation of the bound-constraint differently for each arity
		if (dynamic_cast<const ZeroaryDirectEffect*>(effect)) {
			if (!info.checkValueIsValid(effect->apply())) {
				std::stringstream buffer;
				buffer << "Error: 0-ary effect '" << *effect << "' of action " << action << " produces out-of-bounds variable values";
				throw std::runtime_error(printer() << "Error: 0-ary effect '" << *effect << "' of action " << action << " produces out-of-bounds variable values");

			}
		} else if (const auto* casted = dynamic_cast<const UnaryDirectEffect*>(effect)) {
			constraints.push_back(new UnaryDomainBoundsConstraint(casted, info));
		} else if (const auto* casted = dynamic_cast<const BinaryDirectEffect*>(effect)) {
			constraints.push_back(new BinaryDomainBoundsConstraint(casted, info));
		} 
		else throw std::runtime_error("Shouldn't be processing an action having effects with arity > 2 here - try the GecodeActionManager instead");
	}
}

std::ostream& BinaryDomainBoundsConstraint::print(std::ostream& os) const {
	os << "BinaryDomainBoundsConstraint[" << print::container(print::Helper::name_variables(_scope)) << "]";
	return os;
}

} // namespaces
