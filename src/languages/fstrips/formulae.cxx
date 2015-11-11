
#include <problem_info.hxx>
#include <languages/fstrips/formulae.hxx>
#include <problem.hxx>
#include <utils/utils.hxx>
#include <state.hxx>
#include <utils/logging.hxx>

#include <typeinfo>

namespace fs0 { namespace language { namespace fstrips {


// A small workaround to circumvent the fact that boost containers do not seem to allow initializer lists
typedef RelationalFormula::Symbol AFSymbol;
const std::map<AFSymbol, std::string> RelationalFormula::symbol_to_string{
	{AFSymbol::EQ, "="}, {AFSymbol::NEQ, "!="}, {AFSymbol::LT, "<"}, {AFSymbol::LEQ, "<="}, {AFSymbol::GT, ">"}, {AFSymbol::GEQ, ">="}
};
const std::map<std::string, AFSymbol> RelationalFormula::string_to_symbol(Utils::flip_map(symbol_to_string));


std::vector<Term::cptr> Formula::all_terms() const {
	std::vector<Term::cptr> res;
	for (auto atom:all_atoms()) {
		auto tmp = atom->all_terms();
		res.insert(res.end(), tmp.cbegin(), tmp.cend());		
	}
	return res;
}

std::ostream& Formula::print(std::ostream& os) const { return print(os, Problem::getInfo()); }

unsigned AtomicFormula::nestedness() const {
	unsigned max = 0;
	for (Term::cptr subterm:_subterms) max = std::max(max, subterm->nestedness());
	return max;
}

std::vector<Term::cptr> AtomicFormula::all_terms() const {
	std::vector<Term::cptr> res;
	for (Term::cptr term:_subterms) {
		auto tmp = term->all_terms();
		res.insert(res.end(), tmp.cbegin(), tmp.cend());
	}
	return res;
}

bool AtomicFormula::interpret(const PartialAssignment& assignment, const Binding& binding) const {
	return _satisfied(NestedTerm::interpret_subterms(_subterms, assignment, binding));
}

bool AtomicFormula::interpret(const State& state, const Binding& binding) const {
	return _satisfied(NestedTerm::interpret_subterms(_subterms, state, binding));
}

std::ostream& RelationalFormula::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << *_subterms[0] << " " << RelationalFormula::symbol_to_string.at(symbol()) << " " << *_subterms[1];
	return os;
}

Formula::cptr AtomicFormula::bind(const Binding& binding, const ProblemInfo& info) const {
	// Process the subterms first
	std::vector<ObjectIdx> constant_values;
	std::vector<Term::cptr> processed_subterms = NestedTerm::bind_subterms(_subterms, binding, info, constant_values);
	
	// Create the corresponding relational or external formula object, according to the symbol
	AtomicFormula::cptr processed = clone(processed_subterms);
	
	// Check if we can resolve the value of the formula statically
	if (constant_values.size() == _subterms.size()) {
		auto resolved = processed->interpret({}) ? static_cast<Formula::cptr>(new Tautology) : static_cast<Formula::cptr>(new Contradiction);
		delete processed;
		return resolved;
	}
	
	return processed;
}

Formula::cptr Conjunction::bind(const Binding& binding, const fs0::ProblemInfo& info) const {
	std::vector<AtomicFormula::cptr> conjuncts;
	for (AtomicFormula::cptr c:_conjuncts) {
		auto processed = c->bind(binding, info);
		// Static checks
		if (processed->is_tautology()) { // No need to add the condition, which is always true
			delete processed;
			continue; 
		} else if (processed->is_contradiction()) { // The whole conjunction statically resolves to false
			delete processed;
			for (auto elem:conjuncts) delete elem;
			return new Contradiction;
		}
		auto processed_atomic = dynamic_cast<AtomicFormula::cptr>(processed);
		assert(processed_atomic);
		conjuncts.push_back(processed_atomic);
	}
	
	if (conjuncts.empty()) return new Tautology; // The empty conjunction is a tautology
	
	return new Conjunction(conjuncts);
}


unsigned Conjunction::nestedness() const {
	unsigned max = 0;
	for (auto element:_conjuncts) max = std::max(max, element->nestedness());
	return max;
}

bool Conjunction::interpret(const PartialAssignment& assignment, const Binding& binding) const {
	for (auto elem:_conjuncts) {
		if (!elem->interpret(assignment, binding)) return false;
	}
	return true;
}

bool Conjunction::interpret(const State& state, const Binding& binding) const {
	for (auto elem:_conjuncts) {
		if (!elem->interpret(state, binding)) return false;
	}
	return true;
}

std::ostream& Conjunction::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	for (unsigned i = 0; i < _conjuncts.size(); ++i) {
		os << *_conjuncts.at(i);
		if (i < _conjuncts.size() - 1) os << " and ";
	}
	return os;
}

std::vector<AtomicFormula::cptr> Conjunction::all_atoms() const {
	std::vector<AtomicFormula::cptr> res;
	for (auto elem:_conjuncts) {
		auto tmp = elem->all_atoms();
		res.insert(res.end(), tmp.cbegin(), tmp.cend());
	}
	return res;
}

Formula::cptr ExistentiallyQuantifiedFormula::bind(const Binding& binding, const ProblemInfo& info) const {
	for (const BoundVariable& var:_variables) {
		if (binding.binds(var.getVariableId())) throw std::runtime_error("Wrong binding - Duplicated variable");
	}
	// TODO Check if the binding is a complete binding and thus we can directly return the (variable-free) conjunction 
	// TODO Redesign this mess
	auto bound_subformula = _subformula->bind(binding, info);
	if (dynamic_cast<Tautology::cptr>(bound_subformula)) return bound_subformula;
	else if (dynamic_cast<Contradiction::cptr>(bound_subformula)) return bound_subformula;
	
	auto bound_conjunction = dynamic_cast<Conjunction::cptr>(bound_subformula);
	assert(bound_conjunction);
	
	return new ExistentiallyQuantifiedFormula(_variables, bound_conjunction);
}

//! Prints a representation of the object to the given stream.
std::ostream& ExistentiallyQuantifiedFormula::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "Exists ";
	for (const BoundVariable& var:_variables) {
		os << var << ": " << info.getTypename(var.getType()) << ", ";
	}
	os << "(" << *_subformula << ")";
	return os;
}

template <typename T>
bool ExistentiallyQuantifiedFormula::interpret_rec(const T& assignment, const Binding& binding, unsigned i) const {
	// Base case - all existentially quantified variables have been bound
	if (i == _variables.size()) return _subformula->interpret(assignment, binding);
	
	const ProblemInfo& info = Problem::getInfo();
	BoundVariable variable = _variables.at(i);
	Binding copy(binding);
	//! Otherwise, iterate through all possible assignments to the currently analyzed variable 'i'
	for (ObjectIdx elem:info.getTypeObjects(variable.getType())) {
		copy.set(variable.getVariableId(), elem);
		if (interpret_rec(assignment, copy, i + 1)) return true;
	}
	return false;
}

Formula* Formula::conjunction(const Contradiction* formula) const { return new Contradiction; }

Formula* Tautology::conjunction(const Formula* 								other) const { return other->conjunction(this); }
Formula* Tautology::conjunction(const AtomicFormula* 						other) const { return other->clone(); }
Formula* Tautology::conjunction(const Conjunction* 							other) const { return other->clone(); }
Formula* Tautology::conjunction(const ExistentiallyQuantifiedFormula*		other) const { return other->clone(); }
Formula* Contradiction::conjunction(const Formula* 							other) const { return other->conjunction(this); }
Formula* Contradiction::conjunction(const AtomicFormula* 					other) const { return new Contradiction; }
Formula* Contradiction::conjunction(const Conjunction* 						other) const { return new Contradiction; }
Formula* Contradiction::conjunction(const ExistentiallyQuantifiedFormula*	other) const { return new Contradiction; }
Formula* Conjunction::conjunction(const Formula* 							other) const { return other->conjunction(this); }
Formula* Conjunction::conjunction(const AtomicFormula* 						other) const { throw std::runtime_error("Unimplemented"); }
Formula* Conjunction::conjunction(const ExistentiallyQuantifiedFormula*		other) const { return other->conjunction(this); }
Formula* ExistentiallyQuantifiedFormula::conjunction(const Formula* 							other) const { return other->conjunction(this); }
Formula* ExistentiallyQuantifiedFormula::conjunction(const AtomicFormula* 						other) const { throw std::runtime_error("Unimplemented"); }
Formula* ExistentiallyQuantifiedFormula::conjunction(const ExistentiallyQuantifiedFormula*		other) const { throw std::runtime_error("Unimplemented"); }	

Conjunction* Conjunction::conjunction(const Conjunction::cptr other) const {
	auto all_subterms = Utils::merge(FStripsUtils::clone(_conjuncts), FStripsUtils::clone(other->_conjuncts));
	return new Conjunction(all_subterms);
}

Formula* ExistentiallyQuantifiedFormula::conjunction(const Conjunction::cptr other) const {
	// We simply return the existentially quantified formula that results from conjuncting the LHS subconjunction with the RHS conjunction, with the same quantified variables.
	return new ExistentiallyQuantifiedFormula(_variables, other->conjunction(_subformula));
}


} } } // namespaces
