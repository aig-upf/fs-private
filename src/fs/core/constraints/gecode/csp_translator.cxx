
#include <fs/core/problem_info.hxx>
#include <fs/core/languages/fstrips/language.hxx>
#include <fs/core/constraints/gecode/csp_translator.hxx>
#include <fs/core/constraints/gecode/helper.hxx>
#include <lapkt/tools/logging.hxx>
#include <fs/core/state.hxx>
#include <fs/core/constraints/gecode/gecode_space.hxx>
#include <fs/core/heuristics/relaxed_plan/rpg_index.hxx>
#include <fs/core/utils/printers/helper.hxx>
#include <fs/core/fstrips/language_info.hxx>

namespace fs0 { namespace gecode {

void CSPTranslator::perform_registration() {
	Gecode::IntVarArray intarray(_base_csp, _intvars);
	_base_csp._intvars.update(_base_csp, intarray);

	Gecode::BoolVarArray boolarray(_base_csp, _boolvars);
	_base_csp._boolvars.update(_base_csp, boolarray);
}

unsigned CSPTranslator::add_intvar(const Gecode::IntVar& csp_variable, type_id var_t, VariableIdx planning_variable) {
	assert((unsigned) _intvars.size() == _intvars_idx.size());
	assert((unsigned) _intvars.size() == _intvars_types.size());
	unsigned id = _intvars.size();
	_intvars << csp_variable;
	_intvars_idx.push_back(planning_variable);
	_intvars_types.push_back(var_t);
	return id;
}

unsigned CSPTranslator::add_boolvar(const Gecode::BoolVar& csp_variable) {
	unsigned id = _boolvars.size();
	_boolvars << csp_variable;
	return id;
}

unsigned CSPTranslator::create_bool_variable() {
	return add_boolvar(Helper::createBoolVariable(_base_csp));
}

void CSPTranslator::registerConstant(const fs::Constant* constant) {
	auto it = _registered.find(constant);
	if (it!= _registered.end()) return; // The element was already registered

	object_id o = constant->getValue();
	type_id t = o_type(o);
	
	unsigned id = 0;
	if (t == type_id::bool_t) {
		bool value =  fs0::value<bool>(o);
		id = add_intvar(Gecode::IntVar(_base_csp, value, value), t);
	} else if (t == type_id::object_t || t == type_id::int_t) {
		int value =  fs0::value<int>(o);
		id = add_intvar(Gecode::IntVar(_base_csp, value, value), t);
	} else {
		throw std::runtime_error("Yet to implement CSPs with float constants");
	}

	_registered.insert(it, std::make_pair(constant, id));
}

void CSPTranslator::registerExistentialVariable(const fs::BoundVariable* variable) {
	TypeIdx fstype = variable->getType();
	type_id tid = fs0::fstrips::LanguageInfo::instance().get_type_id(fstype);
	unsigned id = add_intvar(Helper::createTemporaryVariable(_base_csp, fstype), tid);
	auto res = _registered.insert(std::make_pair(variable, id));
	_unused(res);
	assert(res.second); // Make sure the element was not there before
}

unsigned CSPTranslator::registerIntVariable(int min, int max) {
	return add_intvar(Helper::createTemporaryIntVariable(_base_csp, min, max), type_id::int_t);
}


bool CSPTranslator::isRegistered(const fs::Term* variable) const {
	return _registered.find(variable) != _registered.end();
}

void CSPTranslator::registerInputStateVariable(VariableIdx variable) {
	auto it = _input_state_variables.find(variable);
	if (it != _input_state_variables.end()) return; // The state variable was already registered, no need to register it again
	
	type_id tid = ProblemInfo::getInstance().sv_type(variable);
	unsigned id = add_intvar(Helper::createPlanningVariable(_base_csp, variable), tid, variable);
	_input_state_variables.insert(std::make_pair(variable, id));
}

bool CSPTranslator::registerNestedTerm(const fs::NestedTerm* nested) {
	const ProblemInfo& info = ProblemInfo::getInstance();
// 	const fs0::fstrips::LanguageInfo::LanguageInfo& linfo = fs0::fstrips::LanguageInfo::instance();
	unsigned symbol_id = nested->getSymbolId();
	const SymbolData& sdata = info.getSymbolData(symbol_id);
	TypeIdx domain_type = sdata.getCodomainType();
	type_id tid = info.get_type_id(sdata.getCodomainType());
	return registerNestedTerm(nested, domain_type, tid);
}

bool CSPTranslator::registerNestedTerm(const fs::NestedTerm* nested, TypeIdx domain_type, const type_id& codomain_tid) {
	auto it = _registered.find(nested);
	if (it!= _registered.end()) return false; // The element was already registered
	unsigned id = add_intvar(Helper::createTemporaryVariable(_base_csp, domain_type), codomain_tid);

	_registered.insert(it, std::make_pair(nested, id));
	return true;
}

bool CSPTranslator::registerNestedTerm(const fs::NestedTerm* nested, int min, int max) {
	auto it = _registered.find(nested);
	if (it!= _registered.end()) return false; // The element was already registered

	unsigned id = add_intvar(Helper::createTemporaryIntVariable(_base_csp, min, max), type_id::int_t);

	_registered.insert(it, std::make_pair(nested, id));
	return true;
}



unsigned CSPTranslator::resolveVariableIndex(const fs::Term* term) const {
	if (auto sv = dynamic_cast<const fs::StateVariable*>(term)) {
		return resolveInputVariableIndex(sv->getValue());
	}

	auto it = _registered.find(term);
	if(it == _registered.end()) {
		throw UnregisteredStateVariableError(fs0::printer() << "Trying to resolve unregistered term \"" << *term << "\"");
	}
	return it->second;
}

const Gecode::IntVar& CSPTranslator::resolveVariable(const fs::Term* term, const FSGecodeSpace& csp) const {
	return resolveVariableFromIndex(resolveVariableIndex(term), csp);
}

object_id CSPTranslator::resolveValue(const fs::Term* term, const FSGecodeSpace& csp) const {
	unsigned idx = resolveVariableIndex(term);
	return resolveValueFromIndex(idx, csp);
}

const Gecode::IntVar& CSPTranslator::resolveVariableFromIndex(unsigned variable_index, const FSGecodeSpace& csp) const {
	return csp._intvars[variable_index];
}

const Gecode::BoolVar& CSPTranslator::resolveBoolVariableFromIndex(unsigned variable_index, const FSGecodeSpace& csp) const {
    return csp._boolvars[variable_index];
}

object_id CSPTranslator::resolveValueFromIndex(unsigned variable_index, const FSGecodeSpace& csp) const {
	const type_id& t = _intvars_types.at(variable_index);
	return make_object(t, resolveVariableFromIndex(variable_index, csp).val());
}

const Gecode::IntVar& CSPTranslator::resolveInputStateVariable(const FSGecodeSpace& csp, VariableIdx variable) const {
	return csp._intvars[resolveInputVariableIndex(variable)];
}

Gecode::IntVarArgs CSPTranslator::resolveVariables(const std::vector<const fs::Term*>& terms, const FSGecodeSpace& csp) const {
	Gecode::IntVarArgs variables;
	for (const fs::Term* term:terms) {
		variables << resolveVariable(term, csp);
	}
	return variables;
}

std::vector<object_id> CSPTranslator::resolveValues(const std::vector<const fs::Term*>& terms, const FSGecodeSpace& csp) const {
	std::vector<object_id> values;
	for (const fs::Term* term:terms) {
		values.push_back(resolveValue(term, csp));
	}
	return values;
}

std::ostream& CSPTranslator::print(std::ostream& os, const FSGecodeSpace& csp) const {
	const fs0::ProblemInfo& info = ProblemInfo::getInstance();
	os << "Gecode CSP with " << _registered.size() + _input_state_variables.size() << " variables" << std::endl;
	
	os << std::endl << "State Variables: " << std::endl;
	for (auto it:_input_state_variables) {
		os << "\t " << info.getVariableName(it.first) << ": " << csp._intvars[it.second] << std::endl;
	}
	
	os << std::endl << "CSP Variables corresponding to other terms: " << std::endl;
	for (auto it:_registered) {
		os << "\t ";
		os << *(it.first);
		os << ": " << csp._intvars[it.second] << std::endl;
	}

	return os;
}

void CSPTranslator::updateStateVariableDomains(FSGecodeSpace& csp, const RPGIndex& graph) const {
	updateStateVariableDomains(csp, graph.get_domains());
}

void CSPTranslator::
updateStateVariableDomains(FSGecodeSpace& csp, const std::vector<Gecode::IntSet>& domains, bool empty_means_no_constraint) const {
	// Iterate over all the input state variables and constrain them according to the RPG layer
	for (const auto& it:_input_state_variables) {
		VariableIdx variable = it.first;
		const Gecode::IntVar& csp_variable = csp._intvars[it.second];
		const Gecode::IntSet& iset = domains.at(variable);

		if (empty_means_no_constraint && iset.size() == 0) continue;
		Helper::constrainCSPVariable(csp, csp_variable, domains.at(variable));
	}
}

void CSPTranslator::updateStateVariableDomains(FSGecodeSpace& csp, const std::vector<const Gecode::IntSet*>& domains) const {
	// Iterate over all the input state variables and constrain them according to the RPG layer
	for (const auto& it:_input_state_variables) {
		VariableIdx variable = it.first;
		const Gecode::IntVar& csp_variable = csp._intvars[it.second];
        const auto& dom = domains.at(variable);
        if (dom != nullptr) {
            Helper::constrainCSPVariable(csp, csp_variable,
                                         *(domains.at(variable)));
        }
	}
}

void CSPTranslator::updateStateVariableDomains(FSGecodeSpace& csp, const State& state) const {
	// Iterate over all the input state variables and assign them the only possible value dictated by the state.
	for (const auto& it:_input_state_variables) {
		VariableIdx variable = it.first;
		const Gecode::IntVar& csp_variable = csp._intvars[it.second];
		Gecode::rel(csp, csp_variable,  Gecode::IRT_EQ, fs0::value<int>(state.getValue(variable)));
	}
}

PartialAssignment CSPTranslator::buildAssignment(FSGecodeSpace& solution) const {
	PartialAssignment assignment;
	for (const auto& it:_input_state_variables) {
		VariableIdx variable = it.first;
		const Gecode::IntVar& csp_variable = solution._intvars[it.second];
		const type_id& t = _intvars_types.at(it.second);
		assignment.insert(std::make_pair(variable, make_object(t, csp_variable.val())));
	}
	return assignment;
}

VariableIdx CSPTranslator::getPlanningVariable(unsigned csp_var_idx) const {
	return _intvars_idx[csp_var_idx];
}

std::vector<std::pair<unsigned, std::vector<unsigned>>>
CSPTranslator::index_fluents(const std::unordered_set<const fs::Term*>& terms) {
	const ProblemInfo& info = ProblemInfo::getInstance();
	std::vector<std::pair<unsigned, std::vector<unsigned>>> tuple_indexes;

	// Register the indexes of the CSP variables that correspond to all involved nested fluent symbols
	for (auto term:terms) {
		if (const fs::FluentHeadedNestedTerm* nested = dynamic_cast<const fs::FluentHeadedNestedTerm*>(term)) {
			unsigned symbol = nested->getSymbolId();
			std::vector<unsigned> indexes;
			for (const fs::Term* subterm:nested->getSubterms()) {
				indexes.push_back(resolveVariableIndex(subterm));
			}
			
			if (!info.isPredicate(symbol)) { // If we have a functional symbol, we add the value of the term to the end of the tuple
				indexes.push_back(resolveVariableIndex(nested));
			}
			tuple_indexes.push_back(std::make_pair(nested->getSymbolId(), indexes));
		}
	}
	return tuple_indexes;
}

void CSPTranslator::registerReifiedAtom(const fs::AtomicFormula* atom) {
	auto it = _reified_atoms.find(atom);
	if (it!= _reified_atoms.end()) return; // The element was already registered

	unsigned id = create_bool_variable();
	_reified_atoms.insert(it, std::make_pair(atom, id));
}

unsigned CSPTranslator::resolveReifiedAtomVariableIndex(const fs::AtomicFormula* atom) const {
    auto it = _reified_atoms.find(atom);
    if(it == _reified_atoms.end()) {
        throw UnregisteredStateVariableError(fs0::printer() << "Trying to resolve reified variable "
                "for unregistered atom \"" << *atom << "\"");
    }
    return it->second;
}

const Gecode::BoolVar&
CSPTranslator::resolveReifiedAtomVariable(const fs::AtomicFormula* atom, const FSGecodeSpace& csp) const {
    return resolveBoolVariableFromIndex(resolveReifiedAtomVariableIndex(atom), csp);
}


} } // namespaces
