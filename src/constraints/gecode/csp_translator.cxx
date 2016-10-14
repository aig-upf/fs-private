
#include <languages/fstrips/language.hxx>
#include <problem.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <constraints/gecode/helper.hxx>
#include <aptk2/tools/logging.hxx>
#include <state.hxx>
#include <constraints/gecode/gecode_csp.hxx>
#include <heuristics/relaxed_plan/rpg_index.hxx>
#include <utils/printers/helper.hxx>

namespace fs0 { namespace gecode {

void CSPTranslator::perform_registration() {
	Gecode::IntVarArray intarray(_base_csp, _intvars);
	_base_csp._intvars.update(_base_csp, false, intarray);

	Gecode::BoolVarArray boolarray(_base_csp, _boolvars);
	_base_csp._boolvars.update(_base_csp, false, boolarray);
}

unsigned CSPTranslator::add_intvar(Gecode::IntVar csp_variable, VariableIdx planning_variable) {
	assert((unsigned) _intvars.size() == _intvars_idx.size());
	unsigned id = _intvars.size();
	_intvars << csp_variable;
	_intvars_idx.push_back(planning_variable);
	return id;
}

unsigned CSPTranslator::add_boolvar(Gecode::BoolVar csp_variable) {
	unsigned id = _boolvars.size();
	_boolvars << csp_variable;
	return id;
}

unsigned CSPTranslator::create_bool_variable() {
	return add_boolvar(Helper::createBoolVariable(_base_csp));
}

bool CSPTranslator::registerConstant(const fs::Constant* constant) {
	auto it = _registered.find(constant);
	if (it!= _registered.end()) return false; // The element was already registered

	int value = constant->getValue();
	unsigned id = add_intvar(Gecode::IntVar(_base_csp, value, value));

	_registered.insert(it, std::make_pair(constant, id));
	return true;
}

void CSPTranslator::registerExistentialVariable(const fs::BoundVariable* variable) {
	unsigned id = add_intvar(Helper::createTemporaryVariable(_base_csp, variable->getType()));
	auto res = _registered.insert(std::make_pair(variable, id));
	_unused(res);
	assert(res.second); // Make sure the element was not there before
}

unsigned CSPTranslator::registerIntVariable(int min, int max) {
	return add_intvar(Helper::createTemporaryIntVariable(_base_csp, min, max));
}


void CSPTranslator::registerInputStateVariable(VariableIdx variable) {
	auto it = _input_state_variables.find(variable);
	if (it != _input_state_variables.end()) return; // The state variable was already registered, no need to register it again
	
	unsigned id = add_intvar(Helper::createPlanningVariable(_base_csp, variable), variable);
	_input_state_variables.insert(std::make_pair(variable, id));
}

bool CSPTranslator::registerNestedTerm(const fs::NestedTerm* nested) {
	TypeIdx domain_type = ProblemInfo::getInstance().getSymbolData(nested->getSymbolId()).getCodomainType();
	return registerNestedTerm(nested, domain_type);
}

bool CSPTranslator::registerNestedTerm(const fs::NestedTerm* nested, TypeIdx domain_type) {
	auto it = _registered.find(nested);
	if (it!= _registered.end()) return false; // The element was already registered

	unsigned id = add_intvar(Helper::createTemporaryVariable(_base_csp, domain_type));

	_registered.insert(it, std::make_pair(nested, id));
	return true;
}

bool CSPTranslator::registerNestedTerm(const fs::NestedTerm* nested, int min, int max) {
	auto it = _registered.find(nested);
	if (it!= _registered.end()) return false; // The element was already registered

	unsigned id = add_intvar(Helper::createTemporaryIntVariable(_base_csp, min, max));

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

const Gecode::IntVar& CSPTranslator::resolveVariable(const fs::Term* term, const GecodeCSP& csp) const {
	return csp._intvars[resolveVariableIndex(term)];
}

ObjectIdx CSPTranslator::resolveValue(const fs::Term* term, const GecodeCSP& csp) const {
	return resolveVariable(term, csp).val();
}

const Gecode::IntVar& CSPTranslator::resolveVariableFromIndex(unsigned variable_index, const GecodeCSP& csp) const {
	return csp._intvars[variable_index];
}

ObjectIdx CSPTranslator::resolveValueFromIndex(unsigned variable_index, const GecodeCSP& csp) const {
	return resolveVariableFromIndex(variable_index, csp).val();
}

const Gecode::IntVar& CSPTranslator::resolveInputStateVariable(const GecodeCSP& csp, VariableIdx variable) const {
	return csp._intvars[resolveInputVariableIndex(variable)];
}

Gecode::IntVarArgs CSPTranslator::resolveVariables(const std::vector<const fs::Term*>& terms, const GecodeCSP& csp) const {
	Gecode::IntVarArgs variables;
	for (const fs::Term* term:terms) {
		variables << resolveVariable(term, csp);
	}
	return variables;
}

std::vector<ObjectIdx> CSPTranslator::resolveValues(const std::vector<const fs::Term*>& terms, const GecodeCSP& csp) const {
	std::vector<ObjectIdx> values;
	for (const fs::Term* term:terms) {
		values.push_back(resolveValue(term, csp));
	}
	return values;
}

std::ostream& CSPTranslator::print(std::ostream& os, const GecodeCSP& csp) const {
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

void CSPTranslator::updateStateVariableDomains(GecodeCSP& csp, const RPGIndex& graph) const {
	updateStateVariableDomains(csp, graph.get_domains());
}

void CSPTranslator::updateStateVariableDomains(GecodeCSP& csp, const std::vector<Gecode::IntSet>& domains) const {
	// Iterate over all the input state variables and constrain them according to the RPG layer
	for (const auto& it:_input_state_variables) {
		VariableIdx variable = it.first;
		const Gecode::IntVar& csp_variable = csp._intvars[it.second];
		Helper::constrainCSPVariable(csp, csp_variable, domains.at(variable));
	}
}

void CSPTranslator::updateStateVariableDomains(GecodeCSP& csp, const State& state) const {
	// Iterate over all the input state variables and assign them the only possible value dictated by the state.
	for (const auto& it:_input_state_variables) {
		VariableIdx variable = it.first;
		const Gecode::IntVar& csp_variable = csp._intvars[it.second];
		Gecode::rel(csp, csp_variable,  Gecode::IRT_EQ, state.getValue(variable));
	}
}

PartialAssignment CSPTranslator::buildAssignment(GecodeCSP& solution) const {
	PartialAssignment assignment;
	for (const auto& it:_input_state_variables) {
		VariableIdx variable = it.first;
		const Gecode::IntVar& csp_variable = solution._intvars[it.second];
		assignment.insert(std::make_pair(variable, csp_variable.val()));
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




} } // namespaces
