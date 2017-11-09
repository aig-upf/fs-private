
#include <fs/core/problem_info.hxx>
#include <fs/core/languages/fstrips/formulae.hxx>
#include <fs/core/languages/fstrips/terms.hxx>
#include <fs/core/languages/fstrips/builtin.hxx>
#include <fs/core/languages/fstrips/axioms.hxx>
#include <fs/core/problem.hxx>
#include <fs/core/utils//utils.hxx>
#include <fs/core/state.hxx>
#include <lapkt/tools/logging.hxx>
#include <fs/core/fstrips/language_info.hxx>

namespace fs0 { namespace language { namespace fstrips {


// A small workaround to circumvent the fact that boost containers do not seem to allow initializer lists
typedef RelationalFormula::Symbol AFSymbol;
const std::map<AFSymbol, std::string> RelationalFormula::symbol_to_string{
	{AFSymbol::EQ, "="}, {AFSymbol::NEQ, "!="}, {AFSymbol::LT, "<"}, {AFSymbol::LEQ, "<="}, {AFSymbol::GT, ">"}, {AFSymbol::GEQ, ">="}
};
// const std::map<std::string, AFSymbol> RelationalFormula::string_to_symbol(Utils::flip_map(symbol_to_string));


bool Formula::interpret(const PartialAssignment& assignment) const { Binding binding; return interpret(assignment, binding); }
bool Formula::interpret(const State& state) const  { Binding binding; return interpret(state, binding); }


AtomicFormula::~AtomicFormula() {
	for (const auto ptr:_subterms) delete ptr;
}

AtomicFormula* AtomicFormula::clone() const { return clone(Utils::clone(_subterms)); }

bool AtomicFormula::interpret(const PartialAssignment& assignment, Binding& binding) const {
	NestedTerm::interpret_subterms(_subterms, assignment, binding, _interpreted_subterms);
	return _satisfied(_interpreted_subterms);
}

bool AtomicFormula::interpret(const State& state, Binding& binding) const {
	NestedTerm::interpret_subterms(_subterms, state, binding, _interpreted_subterms);
	return _satisfied(_interpreted_subterms);
}

type_id RelationalFormula::
_check_types_are_equal(const object_id& lhs, const object_id& rhs) const {
	type_id t = o_type(lhs);
	if (t  != o_type(rhs)) {
		auto it = symbol_to_string.find(symbol());
		assert(it != symbol_to_string.end());
		std::string sym_string = it->second;
		std::stringstream buffer;
		buffer << "Type mismatch in comparison '";
		buffer << *this;
		buffer << "' lhs '" << lhs << "' type is '" << to_string(o_type(lhs)) << "',";
		buffer << " rhs '" << rhs << "' type is '" << to_string(o_type(rhs)) << "'.";
		throw std::runtime_error( buffer.str() );
	}
	return t;
}

bool RelationalFormula::
_eq_op(const object_id& o1, const object_id& o2) const {
	// If not overriden, the method complains with an exception
	_throw_unsupported_type(o_type(o1));
	return false; // To avoid compiler warnings
}

void RelationalFormula::
_throw_unsupported_type(const type_id& t) const {
	auto it = symbol_to_string.find(symbol());
	assert( it != symbol_to_string.end() );
	std::string sym_string = it->second;
	throw std::runtime_error("Relational operator \"" + sym_string + "\" is not defined for type '" + to_string(t) + "'" );
}


bool
RelationalFormula::_satisfied(const object_id& lhs, const object_id& rhs) const {
	type_id t = _check_types_are_equal(lhs, rhs);

	if (t == type_id::int_t) {
		return _int_handler(fs0::value<int>(lhs), fs0::value<int>(rhs));

	} else if (t == type_id::float_t) {
		return _float_handler(fs0::value<float>(lhs), fs0::value<float>(rhs));

	} else if (t == type_id::object_t || t == type_id::bool_t) {
		// Predicates "=" and "!=" are defined also for bools and objects
		return _eq_op(lhs, rhs);

	}

	_throw_unsupported_type(t);
	return false; // To avoid compiler warnings
}


std::ostream& RelationalFormula::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << *_subterms[0] << " " << RelationalFormula::symbol_to_string.at(symbol()) << " " << *_subterms[1];
	return os;
}


std::ostream& ExternallyDefinedFormula::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << name() << "(";
	for (const auto ptr:_subterms) os << *ptr << ", ";
	os << ")";
	return os;
}


std::ostream& AxiomaticFormula::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << name() << "(";
	for (const auto ptr:_subterms) os << *ptr << ", ";
	os << ")";
	return os;
}

bool AxiomaticFormula::interpret(const PartialAssignment& assignment, Binding& binding) const {
	throw std::runtime_error("UNIMPLEMENTED");
}

bool AxiomaticFormula::interpret(const State& state, Binding& binding) const {
	NestedTerm::interpret_subterms(_subterms, state, binding, _interpreted_subterms);
	return compute(state, _interpreted_subterms);
}


AxiomaticAtom::~AxiomaticAtom() {
	for (const auto ptr:_subterms) delete ptr;
}

AxiomaticAtom::AxiomaticAtom(const AxiomaticAtom& other) :
	_axiom(other._axiom),
	_subterms(Utils::clone(other._subterms))
{}

bool AxiomaticAtom::interpret(const PartialAssignment& assignment, Binding& binding) const {
	std::vector<object_id> _interpreted_subterms;
	NestedTerm::interpret_subterms(_subterms, assignment, binding, _interpreted_subterms);
	Binding axiom_binding(_interpreted_subterms);
	return _axiom->getDefinition()->interpret(assignment, axiom_binding);
}

bool AxiomaticAtom::interpret(const State& state, Binding& binding) const {
	std::vector<object_id> _interpreted_subterms;
	NestedTerm::interpret_subterms(_subterms, state, binding, _interpreted_subterms);
	Binding axiom_binding(_interpreted_subterms);
	return _axiom->getDefinition()->interpret(state, axiom_binding);
}

std::ostream& AxiomaticAtom::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << _axiom->getName() << "(";
	for (const auto ptr:_subterms) os << *ptr << ", ";
	os << ")";
	return os;
}



OpenFormula::OpenFormula(const OpenFormula& other) :
	_subformulae(Utils::clone(other._subformulae))
{}


std::ostream& OpenFormula::
print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << name() << " ( ";
	for (unsigned i = 0; i < _subformulae.size(); ++i) {
		os << *_subformulae.at(i);
		if (i < _subformulae.size() - 1) os << ", ";
	}
	os << " ) ";
	return os;
}

bool Conjunction::
interpret(const PartialAssignment& assignment, Binding& binding) const {
	for (auto elem:_subformulae) {
		if (!elem->interpret(assignment, binding)) return false;
	}
	return true;
}

bool Conjunction::
interpret(const State& state, Binding& binding) const {
	for (auto elem:_subformulae) {
		if (!elem->interpret(state, binding)) return false;
	}
	return true;
}


bool AtomConjunction::
interpret(const State& state) const {
	for (const auto& atom:_atoms) {
		if ( state.getValue(atom.first) != atom.second) return false;
	}
	return true;
}

bool Disjunction::
interpret(const PartialAssignment& assignment, Binding& binding) const {
	for (auto elem:_subformulae) {
		if (elem->interpret(assignment, binding)) return true;
	}
	return false;
}

bool Disjunction::
interpret(const State& state, Binding& binding) const {
	for (auto elem:_subformulae) {
		if (elem->interpret(state, binding)) return true;
	}
	return false;
}


bool Negation::
interpret(const PartialAssignment& assignment, Binding& binding) const {
	return !_subformulae[0]->interpret(assignment, binding);
}

bool Negation::
interpret(const State& state, Binding& binding) const {
	return !_subformulae[0]->interpret(state, binding);
}

QuantifiedFormula::~QuantifiedFormula() {
	delete _subformula;
	for (auto ptr:_variables) delete ptr;
}

QuantifiedFormula::QuantifiedFormula(const QuantifiedFormula& other) :
_variables(Utils::clone(other._variables)), _subformula(other._subformula->clone())
{}

//! Prints a representation of the object to the given stream.
std::ostream& QuantifiedFormula::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << name() << " ";
	for (const BoundVariable* var:_variables) {
		os << *var << ": " << info.getTypename(var->getType()) << " ";
	}
	os << " s.t. ";
	os << "(" << *_subformula << ")";
	return os;
}

bool ExistentiallyQuantifiedFormula::interpret(const PartialAssignment& assignment, Binding& binding) const {
	assert(binding.size()==0); // ATM we do not allow for nested quantifications
	return interpret_rec(assignment, binding, 0);
}

bool ExistentiallyQuantifiedFormula::interpret(const State& state, Binding& binding) const {
	assert(binding.size()==0); // ATM we do not allow for nested quantifications
	return interpret_rec(state, binding, 0);
}

template <typename T>
bool ExistentiallyQuantifiedFormula::interpret_rec(const T& assignment, Binding& binding, unsigned i) const {
	// Base case - all quantified variables have been bound
	if (i == _variables.size()) return _subformula->interpret(assignment, binding);

	const ProblemInfo& info = ProblemInfo::getInstance();
	const BoundVariable* variable = _variables.at(i);
	//! Otherwise, iterate through all possible assignments to the currently analyzed variable 'i'
	for (const object_id& elem:info.getTypeObjects(variable->getType())) {
		binding.set(variable->getVariableId(), elem);
		if (interpret_rec(assignment, binding, i + 1)) return true;
	}
	return false;
}


bool UniversallyQuantifiedFormula::interpret(const PartialAssignment& assignment, Binding& binding) const {
	return interpret_rec(assignment, binding, 0);
}

bool UniversallyQuantifiedFormula::interpret(const State& state, Binding& binding) const {
	return interpret_rec(state, binding, 0);
}

template <typename T>
bool UniversallyQuantifiedFormula::interpret_rec(const T& assignment, Binding& binding, unsigned i) const {
	// Base case - all quantified variables have been bound
	if (i == _variables.size()) return _subformula->interpret(assignment, binding);

	const ProblemInfo& info = ProblemInfo::getInstance();
	const BoundVariable* variable = _variables.at(i);
	//! Otherwise, iterate through all possible assignments to the currently analyzed variable 'i'
	for (const object_id& elem:info.getTypeObjects(variable->getType())) {
		binding.set(variable->getVariableId(), elem);
		if (!interpret_rec(assignment, binding, i + 1)) return false;
	}
	return true;
}

std::vector<const AtomicFormula*> check_all_atomic_formulas(const std::vector<const Formula*> formulas) {
	std::vector<const AtomicFormula*> downcasted;
	for (const auto formula:formulas) {
		const fs::AtomicFormula* sub = dynamic_cast<const fs::AtomicFormula*>(formula);
		if (!sub) throw std::runtime_error("Only conjunctions of atoms supported so far");
		downcasted.push_back(sub);
	}
	return downcasted;
}

EQAtomicFormula::EQAtomicFormula(const std::vector<const Term*>& subterms)
	: RelationalFormula(subterms) {

	_int_handler = [](int lhs, int rhs) { return lhs == rhs; };
	_float_handler = [](float lhs, float rhs) { return std::fabs(lhs - rhs) <= FP_TOLERANCE; };
}

NEQAtomicFormula::NEQAtomicFormula(const std::vector<const Term*>& subterms)
	: RelationalFormula(subterms) {

	_int_handler = [](int lhs, int rhs) { return lhs != rhs; };
	_float_handler = [](float lhs, float rhs) { return std::fabs(lhs - rhs) > FP_TOLERANCE; };
}

LTAtomicFormula::LTAtomicFormula(const std::vector<const Term*>& subterms)
	: RelationalFormula(subterms) {

	_int_handler = [](int lhs, int rhs) { return lhs < rhs; };
	_float_handler = [](float lhs, float rhs) { return lhs < rhs; };
}

LEQAtomicFormula::LEQAtomicFormula(const std::vector<const Term*>& subterms)
	: RelationalFormula(subterms) {

	_int_handler = [](int lhs, int rhs) { return lhs <= rhs; };
	_float_handler = [](float lhs, float rhs) { return lhs <= rhs; };
}


std::vector< RelationalFormula* >
LEQAtomicFormula::relax( const Constant& slack ) const {
	auto slacked_term = new AdditionTerm( {getSubterms()[1]->clone(), slack.clone()} );
	std::vector< const Term* > st;
	try {
		object_id value = slacked_term->interpret(PartialAssignment());
		auto slack_constant = new Constant(value,  fs0::fstrips::LanguageInfo::instance().get_fstype_id("number"));
		st = { getSubterms()[0]->clone(), slack_constant };
	} catch(...) {
		st = { getSubterms()[0]->clone(), slacked_term };
	}
    return { new LEQAtomicFormula(st)};
}

GTAtomicFormula::GTAtomicFormula(const std::vector<const Term*>& subterms)
	: RelationalFormula(subterms) {

	_int_handler = [](int lhs, int rhs) { return lhs > rhs; };
	_float_handler = [](float lhs, float rhs) { return lhs > rhs; };
}

GEQAtomicFormula::GEQAtomicFormula(const std::vector<const Term*>& subterms)
	: RelationalFormula(subterms) {

	_int_handler = [](int lhs, int rhs) { return lhs >= rhs; };
	_float_handler = [](float lhs, float rhs) { return lhs >= rhs; };
}


std::vector< RelationalFormula* >
GEQAtomicFormula::relax( const Constant& slack ) const {
	auto slacked_term = new SubtractionTerm( {getSubterms()[1]->clone(), slack.clone()} );
	std::vector< const Term* > st;
	try {
		object_id value = slacked_term->interpret({});
		auto slack_constant = new Constant(value, fs0::fstrips::LanguageInfo::instance().get_fstype_id("number"));
		st = { getSubterms()[0]->clone(), slack_constant };
	} catch(...) {
		st = { getSubterms()[0]->clone(), slacked_term };
	}
    return { new GEQAtomicFormula(st)};
}

} } } // namespaces
