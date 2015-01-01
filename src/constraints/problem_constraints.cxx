

#include <constraints/problem_constraints.hxx>
#include <utils/projections.hxx>

namespace aptk { namespace core {

ProblemConstraint::ProblemConstraint(Constraint* constraint, const VariableIdxVector& variables) 
	: ctr(constraint), _scope(variables) {}

ProblemConstraint::~ProblemConstraint() {
	delete ctr;
}

//! Returns true iff the current constraint is satisfied in the given state.
bool ProblemConstraint::isSatisfied(const State& s) const {
	ObjectIdxVector projection = Projections::project(s, _scope);
	return ctr->isSatisfied(projection);
}

//! Filters from the set of currently loaded projections
Constraint::Output ProblemConstraint::filter(unsigned variable) {
	return ctr->filter(current_projection);
}

//! Filters from a new set of domains.
Constraint::Output ProblemConstraint::filter(const DomainMap& domains) {
	DomainVector projection = Projections::project(domains, _scope);
	return ctr->filter(projection);
}	

//! Loads (i.e. caches a pointer of) the domain projections of the given state
void ProblemConstraint::loadDomains(const DomainMap& domains) {
	current_projection = Projections::project(domains, _scope);
}

} } // namespaces

