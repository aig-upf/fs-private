
#include <problem_info.hxx>
#include <languages/fstrips/formulae.hxx>
#include <languages/fstrips/terms.hxx>
#include <problem.hxx>
#include <utils/utils.hxx>
#include <state.hxx>
#include <aptk2/tools/logging.hxx>
#include <utils/binding.hxx>

namespace fs0 { namespace language { namespace fstrips {


// A small workaround to circumvent the fact that boost containers do not seem to allow initializer lists
typedef RelationalFormula::Symbol AFSymbol;
const std::map<AFSymbol, std::string> RelationalFormula::symbol_to_string{
	{AFSymbol::EQ, "="}, {AFSymbol::NEQ, "!="}, {AFSymbol::LT, "<"}, {AFSymbol::LEQ, "<="}, {AFSymbol::GT, ">"}, {AFSymbol::GEQ, ">="}
};
// const std::map<std::string, AFSymbol> RelationalFormula::string_to_symbol(Utils::flip_map(symbol_to_string));


std::vector<const Term*> Formula::all_terms() const {
	std::vector<const Term*> res;
	for (auto atom:all_atoms()) {
		auto tmp = atom->all_terms();
		res.insert(res.end(), tmp.cbegin(), tmp.cend());
	}
	return res;
}

std::vector<const AtomicFormula*> Formula::all_atoms() const {
	return Utils::filter_by_type<const AtomicFormula*>(all_formulae());
}

bool Formula::interpret(const PartialAssignment& assignment) const { return interpret(assignment, Binding::EMPTY_BINDING); }
bool Formula::interpret(const State& state) const  { return interpret(state, Binding::EMPTY_BINDING); }


std::ostream& Formula::print(std::ostream& os) const { return print(os, ProblemInfo::getInstance()); }

unsigned AtomicFormula::nestedness() const {
	unsigned max = 0;
	for (const Term* subterm:_subterms) max = std::max(max, subterm->nestedness());
	return max;
}

AtomicFormula::~AtomicFormula() {
	for (const auto ptr:_subterms) delete ptr;
}

std::vector<const Term*> AtomicFormula::all_terms() const {
	std::vector<const Term*> res;
	for (const Term* term:_subterms) {
		auto tmp = term->all_terms();
		res.insert(res.end(), tmp.cbegin(), tmp.cend());
	}
	return res;
}

AtomicFormula* AtomicFormula::clone() const { return clone(Utils::clone(_subterms)); }

bool AtomicFormula::interpret(const PartialAssignment& assignment, const Binding& binding) const {
	NestedTerm::interpret_subterms(_subterms, assignment, binding, _interpreted_subterms);
	return _satisfied(_interpreted_subterms);
}

bool AtomicFormula::interpret(const State& state, const Binding& binding) const {
	NestedTerm::interpret_subterms(_subterms, state, binding, _interpreted_subterms);
	return _satisfied(_interpreted_subterms);
}

std::ostream& RelationalFormula::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << *_subterms[0] << " " << RelationalFormula::symbol_to_string.at(symbol()) << " " << *_subterms[1];
	return os;
}

const Formula* AtomicFormula::bind(const Binding& binding, const ProblemInfo& info) const {
	// Process the subterms first
	std::vector<ObjectIdx> constant_values;
	std::vector<const Term*> processed_subterms = NestedTerm::bind_subterms(_subterms, binding, info, constant_values);

	// Create the corresponding relational or external formula object, according to the symbol
	const AtomicFormula* processed = clone(processed_subterms);

	// Check if we can resolve the value of the formula statically
	if (constant_values.size() == _subterms.size()) {
		auto resolved = processed->interpret({}) ? static_cast<const Formula*>(new Tautology) : static_cast<const Formula*>(new Contradiction);
		delete processed;
		return resolved;
	}

	return processed;
}

Conjunction::Conjunction(const Conjunction& other) :
	_conjuncts(Utils::clone(other._conjuncts))
{}
	
const Formula* Conjunction::bind(const Binding& binding, const fs0::ProblemInfo& info) const {
	std::vector<const AtomicFormula*> conjuncts;
	std::vector<AtomConjunction::AtomT> atoms;
	for (const AtomicFormula* c:_conjuncts) {
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
		auto processed_atomic = dynamic_cast<const AtomicFormula*>(processed);
		assert(processed_atomic);
		conjuncts.push_back(processed_atomic);
		const EQAtomicFormula* cc = dynamic_cast<const EQAtomicFormula*>(c);
		if( cc == nullptr ) continue;
		const StateVariable* lhs = dynamic_cast<const StateVariable*>(cc->lhs());
		if( lhs == nullptr ) continue;
		const Constant* rhs = dynamic_cast< const Constant*>(cc->rhs());
		if( rhs == nullptr ) continue;
		atoms.push_back(std::make_pair(lhs->getValue(), rhs->getValue()));
	}

	if (conjuncts.empty()) return new Tautology; // The empty conjunction is a tautology

	if ( conjuncts.size() == atoms.size()) { // All the subformulae are atoms
		return new AtomConjunction( conjuncts, atoms );
	}

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

std::vector<const Formula*> Conjunction::all_formulae() const {
	std::vector<const Formula*> res(1, this);
	for (auto elem:_conjuncts) {
		auto tmp = elem->all_formulae();
		res.insert(res.end(), tmp.cbegin(), tmp.cend());
	}
	return res;
}



bool
AtomConjunction::interpret(const State& state) const {
	for (const auto& atom:_atoms) {
		if ( state.getValue(atom.first) != atom.second) return false;
	}
	return true;
}


ExistentiallyQuantifiedFormula::ExistentiallyQuantifiedFormula(const ExistentiallyQuantifiedFormula& other) :
_variables(Utils::clone(other._variables)), _subformula(other._subformula->clone())
{}

std::vector<const Formula*> ExistentiallyQuantifiedFormula::all_formulae() const {
	std::vector<const Formula*> res(1, this);
	auto tmp = _subformula->all_formulae();
	res.insert(res.end(), tmp.cbegin(), tmp.cend());
	return res;
}

const Formula* ExistentiallyQuantifiedFormula::bind(const Binding& binding, const ProblemInfo& info) const {
	// Check that the provided binding is not binding a variable which is actually re-bound again by the current existential quantifier
	for (const BoundVariable* var:_variables) {
		if (binding.binds(var->getVariableId())) throw std::runtime_error("Wrong binding - Duplicated variable");
	}
	// TODO Check if the binding is a complete binding and thus we can directly return the (variable-free) conjunction
	// TODO Redesign this mess
	auto bound_subformula = _subformula->bind(binding, info);
	if (dynamic_cast<const Tautology*>(bound_subformula) || dynamic_cast<const Contradiction*>(bound_subformula)) return bound_subformula;

	auto bound_conjunction = dynamic_cast<const Conjunction*>(bound_subformula);
	assert(bound_conjunction);

	return new ExistentiallyQuantifiedFormula(_variables, bound_conjunction);
}

//! Prints a representation of the object to the given stream.
std::ostream& ExistentiallyQuantifiedFormula::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "Exists ";
	for (const BoundVariable* var:_variables) {
		os << *var << ": " << info.getTypename(var->getType()) << ", ";
	}
	os << "(" << *_subformula << ")";
	return os;
}

bool ExistentiallyQuantifiedFormula::interpret(const PartialAssignment& assignment, const Binding& binding) const {
	assert(binding.size()==0); // ATM we do not allow for nested quantifications
	return interpret_rec(assignment, Binding(_variables.size()), 0);
}

bool ExistentiallyQuantifiedFormula::interpret(const State& state, const Binding& binding) const {
	assert(binding.size()==0); // ATM we do not allow for nested quantifications
	return interpret_rec(state, Binding(_variables.size()), 0);
}

template <typename T>
bool ExistentiallyQuantifiedFormula::interpret_rec(const T& assignment, const Binding& binding, unsigned i) const {
	// Base case - all existentially quantified variables have been bound
	if (i == _variables.size()) return _subformula->interpret(assignment, binding);

	const ProblemInfo& info = ProblemInfo::getInstance();
	const BoundVariable* variable = _variables.at(i);
	Binding copy(binding);
	//! Otherwise, iterate through all possible assignments to the currently analyzed variable 'i'
	for (ObjectIdx elem:info.getTypeObjects(variable->getType())) {
		copy.set(variable->getVariableId(), elem);
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

Conjunction* Conjunction::conjunction(const Conjunction* other) const {
	auto all_subterms = Utils::merge(Utils::clone(_conjuncts), Utils::clone(other->_conjuncts));
	return new Conjunction(all_subterms);
}

Formula* ExistentiallyQuantifiedFormula::conjunction(const Conjunction* other) const {
	// We simply return the existentially quantified formula that results from conjuncting the LHS subconjunction with the RHS conjunction, with the same quantified variables.
	return new ExistentiallyQuantifiedFormula(_variables, other->conjunction(_subformula));
}


} } } // namespaces
