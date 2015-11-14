
#include <problem.hxx>
#include <problem_info.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <constraints/gecode/helper.hxx>
#include <constraints/gecode/translators/nested_fluent.hxx>
#include <utils/logging.hxx>
#include <constraints/gecode/rpg_layer.hxx>

namespace fs0 { namespace gecode {

void GecodeCSPVariableTranslator::perform_registration() {
	IntVarArray intarray(_base_csp, _intvars);
	_base_csp._intvars.update(_base_csp, false, intarray);

	BoolVarArray boolarray(_base_csp, _boolvars);
	_base_csp._boolvars.update(_base_csp, false, boolarray);	
}

unsigned GecodeCSPVariableTranslator::add_intvar(Gecode::IntVar csp_variable, VariableIdx planning_variable) {
	assert(_intvars.size() == _intvars_idx.size());
	unsigned id = _intvars.size();
	_intvars << csp_variable;
	_intvars_idx.push_back(planning_variable);
	return id;
}

unsigned GecodeCSPVariableTranslator::add_boolvar(Gecode::BoolVar csp_variable) {
	unsigned id = _boolvars.size();
	_boolvars << csp_variable;
	return id;
}

unsigned GecodeCSPVariableTranslator::create_bool_variable() {
	return add_boolvar(Helper::createBoolVariable(_base_csp));
}

bool GecodeCSPVariableTranslator::registerConstant(fs::Constant::cptr constant) {
	TranslationKey key(constant, CSPVariableType::Input); // Constants are always considered as input variables

	auto it = _registered.find(key);
	if (it!= _registered.end()) return false; // The element was already registered

	int value = constant->getValue();
	unsigned id = add_intvar(Gecode::IntVar(_base_csp, value, value));

	_registered.insert(it, std::make_pair(key, id));
	return true;
}

void GecodeCSPVariableTranslator::registerExistentialVariable(fs::BoundVariable::cptr variable) {
	TranslationKey key(variable, CSPVariableType::Input); // Constants are always considered as input variables
	unsigned id = add_intvar(Helper::createTemporaryVariable(_base_csp, variable->getType()));
	auto res = _registered.insert(std::make_pair(key, id));
	assert(res.second); // Make sure the element was not there before
}


void GecodeCSPVariableTranslator::registerInputStateVariable(VariableIdx variable, bool nullable) {
	auto it = _input_state_variables.find(variable);
	if (it != _input_state_variables.end()) { // The state variable was already registered, no need to register it again
		assert(nullable == it->second.second); // We just check that we're not trying to register it with a different 'nullable' value, which should never happen
		return; 
	}

	unsigned id = add_intvar(Helper::createPlanningVariable(_base_csp, variable, nullable), variable);
	_input_state_variables.insert(std::make_pair(variable, std::make_pair(id, nullable)));
}

bool GecodeCSPVariableTranslator::registerNestedTerm(fs::NestedTerm::cptr nested, CSPVariableType type) {
	TypeIdx domain_type = Problem::getInfo().getFunctionData(nested->getSymbolId()).getCodomainType();
	return registerNestedTerm(nested, type, domain_type);
}

bool GecodeCSPVariableTranslator::registerNestedTerm(fs::NestedTerm::cptr nested, CSPVariableType type, TypeIdx domain_type) {
	TranslationKey key(nested, type);
	auto it = _registered.find(key);
	if (it!= _registered.end()) return false; // The element was already registered

	unsigned id = add_intvar(Helper::createTemporaryVariable(_base_csp, domain_type));

	_registered.insert(it, std::make_pair(key, id));
	return true;
}

bool GecodeCSPVariableTranslator::registerNestedTerm(fs::NestedTerm::cptr nested, CSPVariableType type, int min, int max) {
	TranslationKey key(nested, type);
	auto it = _registered.find(key);
	if (it!= _registered.end()) return false; // The element was already registered

	unsigned id = add_intvar(Helper::createTemporaryIntVariable(_base_csp, min, max));

	_registered.insert(it, std::make_pair(key, id));
	return true;
}


unsigned GecodeCSPVariableTranslator::resolveVariableIndex(fs::Term::cptr term, CSPVariableType type) const {
	if (auto sv = dynamic_cast<StateVariable::cptr>(term)) {
		assert(type == CSPVariableType::Input);  // We're not supporting Output state variables right now, but adding support for them, if needed, would be straight-forward
		return resolveInputVariableIndex(sv->getValue());
	}

	auto it = _registered.find(TranslationKey(term, type));
	if(it == _registered.end()) {
		throw UnregisteredStateVariableError("Trying to translate a non-existing CSP variable");
	}
	return it->second;
}

const Gecode::IntVar& GecodeCSPVariableTranslator::resolveVariable(fs::Term::cptr term, CSPVariableType type, const SimpleCSP& csp) const {
	return csp._intvars[resolveVariableIndex(term, type)];
}

ObjectIdx GecodeCSPVariableTranslator::resolveValue(fs::Term::cptr term, CSPVariableType type, const SimpleCSP& csp) const {
	return resolveVariable(term, type, csp).val();
}

ObjectIdx GecodeCSPVariableTranslator::resolveValueFromIndex(unsigned variable_index, const SimpleCSP& csp) const {
	return csp._intvars[variable_index].val();
}


Gecode::IntVarArgs GecodeCSPVariableTranslator::resolveVariables(const std::vector<fs::Term::cptr>& terms, CSPVariableType type, const SimpleCSP& csp) const {
	Gecode::IntVarArgs variables;
	for (const Term::cptr term:terms) {
		variables << resolveVariable(term, type, csp);
	}
	return variables;
}

std::ostream& GecodeCSPVariableTranslator::print(std::ostream& os, const SimpleCSP& csp) const {
	const fs0::ProblemInfo& info = Problem::getInfo();
	os << "Gecode CSP with " << _registered.size() + _input_state_variables.size() << " variables" << std::endl;
	
	os << std::endl << "State Variables (direct and derived): " << std::endl;
	for (auto it:_input_state_variables) {
		os << "\t " << info.getVariableName(it.first) << (it.second.second ? " (nullable)": "") << ": " << csp._intvars[it.second.first] << std::endl;
	}
	
	os << std::endl << "CSP Variables corresponding to other terms: " << std::endl;
	for (auto it:_registered) {
		os << "\t ";
		os << *(it.first.getTerm());
		if (it.first.getType() == CSPVariableType::Output) os << "'"; // We simply mark output variables with a "'"
		os << ": " << csp._intvars[it.second] << std::endl;
	}

	return os;
}

void GecodeCSPVariableTranslator::updateStateVariableDomains(SimpleCSP& csp, const GecodeRPGLayer& layer) const {
	// Iterate over all the input state variables and constrain them according to the RPG layer
	for (const auto& it:_input_state_variables) {
		VariableIdx variable = it.first;
		const Gecode::IntVar& csp_variable = csp._intvars[it.second.first];
		bool nullable = it.second.second;
		Helper::constrainCSPVariable(csp, csp_variable, layer.get_domain(variable), nullable);
	}
}

void GecodeCSPVariableTranslator::updateStateVariableDomains(SimpleCSP& csp, const State& state) const {
	// Iterate over all the input state variables and assign them the only possible value dictated by the state.
	for (const auto& it:_input_state_variables) {
		VariableIdx variable = it.first;
		const Gecode::IntVar& csp_variable = csp._intvars[it.second.first];
		Gecode::rel(csp, csp_variable,  Gecode::IRT_EQ, state.getValue(variable));
	}
}

PartialAssignment GecodeCSPVariableTranslator::buildAssignment(SimpleCSP& solution) const {
	PartialAssignment assignment;
	for (const auto& it:_input_state_variables) {
		VariableIdx variable = it.first;
		const Gecode::IntVar& csp_variable = solution._intvars[it.second.first];
		assignment.insert(std::make_pair(variable, csp_variable.val()));
	}
	return assignment;
}

VariableIdx GecodeCSPVariableTranslator::getPlanningVariable(unsigned csp_var_idx) const {
	return _intvars_idx[csp_var_idx];
}

} } // namespaces
