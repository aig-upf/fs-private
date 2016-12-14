

#include <search/drivers/sbfws/iw_run.hxx>

namespace fs0 { namespace bfws {

//! A helper to derive the distinct goal atoms
/*
const std::vector<const fs::AtomicFormula*>&
obtain_goal_atoms(const fs::Formula* goal) {
	const fs::Conjunction* conjunction = dynamic_cast<const fs::Conjunction*>(goal);
	if (!conjunction) {
		throw std::runtime_error("This search mode can only be applied to problems featuring simple goal conjunctions");
	}
	return conjunction->getConjuncts();
}
*/


//! A helper to derive the distinct goal atoms
std::vector<Atom>
obtain_goal_atoms(const fs::Formula* goal) {
	const fs::Conjunction* conjunction = dynamic_cast<const fs::Conjunction*>(goal);
	if (!conjunction) {
		throw std::runtime_error("This search mode can only be applied to problems featuring simple goal conjunctions");
	}
	
	std::vector<Atom> goal_atoms;
	
	for (const fs::AtomicFormula* atom:conjunction->getConjuncts()) {
		auto eq =  dynamic_cast<const fs::EQAtomicFormula*>(atom);
		if (!eq) { // This could be easily extended to negated atoms
			throw std::runtime_error("This search mode can only be applied to problems featuring simple goal conjunctions");
		}
		
		auto sv =  dynamic_cast<const fs::StateVariable*>(eq->lhs());
		if (!sv) throw std::runtime_error("This search mode can only be applied to problems featuring simple goal conjunctions");
		
		auto ct =  dynamic_cast<const fs::Constant*>(eq->rhs());
		if (!ct) throw std::runtime_error("This search mode can only be applied to problems featuring simple goal conjunctions");
		
		goal_atoms.push_back(Atom(sv->getValue(), ct->getValue()));
	}
	
	return goal_atoms;
}

} } // namespaces
