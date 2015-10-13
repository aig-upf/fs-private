
#include <problem.hxx>
#include <problem_info.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <constraints/gecode/helper.hxx>
#include <constraints/gecode/translators/nested_fluent.hxx>
#include <utils/logging.hxx>
#include <constraints/gecode/rpg_layer.hxx>

namespace fs0 { namespace gecode {

bool GecodeCSPVariableTranslator::isRegistered(const fs::Term::cptr term, CSPVariableType type) const {
	return _registered.find(TranslationKey(term, type)) != _registered.end();
}

bool GecodeCSPVariableTranslator::isPosted(const fs::Term::cptr term, CSPVariableType type) const {
	return _posted.find(TranslationKey(term, type)) != _posted.end();
}

void GecodeCSPVariableTranslator::setPosted(const fs::Term::cptr term, CSPVariableType type) {
	auto res = _posted.insert(TranslationKey(term, type));
	assert(res.second); // If the element had already been posted, there is some bug.
	_unused(res);
}

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

bool GecodeCSPVariableTranslator::registerConstant(fs::Constant::cptr constant) {
	TranslationKey key(constant, CSPVariableType::Input); // Constants are always considered as input variables

	auto it = _registered.find(key);
	if (it!= _registered.end()) return false; // The element was already registered

	int value = constant->getValue();
	unsigned id = add_intvar(Gecode::IntVar(_base_csp, value, value));

	_registered.insert(it, std::make_pair(key, id));
	return true;
}

bool GecodeCSPVariableTranslator::registerStateVariable(fs::StateVariable::cptr variable, CSPVariableType type) {
	TranslationKey key(variable, type);
	auto it = _registered.find(key);
	if (it!=_registered.end()) return false; // The element was already registered

	unsigned variable_id = variable->getValue();

	//  TODO we need to think what to do in these cases where a derived state variable is also used as a primary state variable in some other expression.
	assert(_derived.find(variable_id) == _derived.end());

	unsigned id = add_intvar(Helper::createPlanningVariable(_base_csp, variable_id), variable_id);

	_registered.insert(it, std::make_pair(key, id));

	// We now cache state variables in different data structures to allow for a more performant subsequent retrieval
	if (type == CSPVariableType::Input) {
		_input_state_variables.insert(std::make_pair(variable_id, id));
	} else if (type == CSPVariableType::Output) {
		_output_state_variables.insert(std::make_pair(variable_id, id));
	}
	return true;
}

bool GecodeCSPVariableTranslator::registerDerivedStateVariable(VariableIdx variable, CSPVariableType type) {
	// TODO We need to check that the state variable wasn't alread registered as a primary state variable
	auto sv = new StateVariable(variable);
	assert(_registered.find(TranslationKey(sv, type)) == _registered.end());
	delete sv; // Yes, this is ugly, but temporary


	// TODO we need to think what to do in these cases where two derived state variables come into play at the same time.
	assert(_derived.find(variable) == _derived.end());

	unsigned id = add_intvar(Helper::createPlanningVariable(_base_csp, variable, true), variable);
	
	_derived.insert(std::make_pair(variable, id));
	return true;
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

void GecodeCSPVariableTranslator::registerNestedFluent(fs::NestedTerm::cptr term, unsigned table_size) {
	TranslationKey key(term, CSPVariableType::Input); // Index variables are always considered input variables
	auto it = element_constraint_data.find(key);
	if (it != element_constraint_data.end()) return; // We already registered an index variable for this particular nested term

	unsigned index_var_idx = add_intvar(Helper::createTemporaryIntVariable(_base_csp, 0, table_size-1));

	unsigned bool_var_idx = _boolvars.size();
	// We create one boolean variable for each of the elements of the disjunction IDX=i \lor TABLE_i = DONT_CARE
	for (unsigned i = 0; i < table_size*2; ++i) {
		_boolvars << Helper::createBoolVariable(_base_csp);
	}

	element_constraint_data.insert(std::make_pair(key, NestedFluentData(index_var_idx, bool_var_idx, table_size)));
}

NestedFluentData& GecodeCSPVariableTranslator::resolveNestedFluent(fs::Term::cptr term) {
	// To avoid code repetition, we call the const version inside the non-const version and then cast the const result back to non-const
	return const_cast<NestedFluentData&>(static_cast<const GecodeCSPVariableTranslator*>(this)->resolveNestedFluent(term));
}

const NestedFluentData& GecodeCSPVariableTranslator::resolveNestedFluent(fs::Term::cptr term) const {
	auto it = element_constraint_data.find(TranslationKey(term, CSPVariableType::Input));  // Index variables always considered input variables by default
	if (it == element_constraint_data.end()) throw std::runtime_error( "Could not resolve indirection for the term provided" );
	return it->second;
}

unsigned GecodeCSPVariableTranslator::resolveVariableIndex(fs::Term::cptr term, CSPVariableType type) const {
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
	os << "Gecode CSP with " << _registered.size() << " variables: " << std::endl;
	for (auto it:_registered) {
		os << "\t ";
		os << *(it.first.getTerm());
		if (it.first.getType() == CSPVariableType::Output) os << "'"; // We simply mark output variables with a "'"
		os << ": " << csp._intvars[it.second] << std::endl;
	}

	os << std::endl << "Plus the following derived state variables: " << std::endl;
	for (auto it:_derived) {
		os << "\t " << info.getVariableName(it.first) << ": " << csp._intvars[it.second] << std::endl;
	}

	os << std::endl << "Plus \"index\" and reification variables for the following terms: " << std::endl;
	for (auto it:element_constraint_data) {
		os << "\t ";
		os << *(it.first.getTerm());
		if (it.first.getType() == CSPVariableType::Output) os << "'"; // We simply mark output variables with a "'"
		os << ": " << it.second.getIndex(csp) << std::endl;
	}

	return os;
}

void GecodeCSPVariableTranslator::updateStateVariableDomains(SimpleCSP& csp, const GecodeRPGLayer& layer) const {
	
	// Iterate over all the input state variables and constrain them according to the RPG layer
	for (const auto& it:_input_state_variables) {
		VariableIdx variable = it.first;
		const Gecode::IntVar& csp_variable = csp._intvars[it.second];
		Helper::constrainCSPVariable(csp, csp_variable, layer.get_domain(variable));
	}

	// Now constrain the derived variables, but not excluding the DONT_CARE value
	for (const auto& it:_derived) {
		VariableIdx variable = it.first;
		const Gecode::IntVar& csp_variable = csp._intvars[it.second];
		Helper::constrainCSPVariable(csp, csp_variable, layer.get_domain(variable), true);
	}
}


unsigned GecodeCSPVariableTranslator::create_bool_variable() {
	unsigned id = _boolvars.size();
	_boolvars << Helper::createBoolVariable(_base_csp);
	return id;
}

PartialAssignment GecodeCSPVariableTranslator::buildAssignment(SimpleCSP& solution) const {
	PartialAssignment assignment;
	for (const auto& it:_input_state_variables) {
		VariableIdx variable = it.first;
		const Gecode::IntVar& csp_variable = solution._intvars[it.second];
		assignment.insert(std::make_pair(variable, csp_variable.val()));
	}
	return assignment;
}

VariableIdx GecodeCSPVariableTranslator::getPlanningVariable(unsigned csp_var_idx) const {
	return _intvars_idx[csp_var_idx];
}

} } // namespaces
