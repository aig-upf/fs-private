
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

bool GecodeCSPVariableTranslator::registerConstant(fs::Constant::cptr constant, SimpleCSP& csp, Gecode::IntVarArgs& variables) {
	TranslationKey key(constant, CSPVariableType::Input); // Constants are always considered as input variables

	auto it = _registered.find(key);
	if (it!= _registered.end()) return false; // The element was already registered

	unsigned id = variables.size();
	int value = constant->getValue();
	variables << Gecode::IntVar(csp, value, value);

	_registered.insert(it, std::make_pair(key, id));
	return true;
}

bool GecodeCSPVariableTranslator::registerStateVariable(fs::StateVariable::cptr variable, CSPVariableType type, SimpleCSP& csp, Gecode::IntVarArgs& intvars, Gecode::BoolVarArgs& boolvars) {
	TranslationKey key(variable, type);
	auto it = _registered.find(key);
	if (it!=_registered.end()) return false; // The element was already registered

	unsigned variable_id = variable->getValue();

	//  TODO we need to think what to do in these cases where a derived state variable is also used as a primary state variable in some other expression.
	assert(_derived.find(variable_id) == _derived.end());

	unsigned id = intvars.size();
	intvars << Helper::createPlanningVariable(csp, variable_id);
	
	unsigned reified_id = boolvars.size();
	boolvars << Helper::createBoolVariable(csp);

	_registered.insert(it, std::make_pair(key, id));

	// We now cache state variables in different data structures to allow for a more performant subsequent retrieval
	if (type == CSPVariableType::Input) {
		_input_state_variables.insert(std::make_pair(variable_id, std::make_pair(id, reified_id)));
	} else if (type == CSPVariableType::Output) {
		_output_state_variables.insert(std::make_pair(variable_id, id));
	}
	return true;
}

bool GecodeCSPVariableTranslator::registerDerivedStateVariable(VariableIdx variable, CSPVariableType type, SimpleCSP& csp, Gecode::IntVarArgs& intvars, Gecode::BoolVarArgs& boolvars) {
	// TODO We need to check that the state variable wasn't alread registered as a primary state variable
	auto sv = new StateVariable(variable);
	assert(_registered.find(TranslationKey(sv, type)) == _registered.end());
	delete sv; // Yes, this is ugly, but temporary


	// TODO we need to think what to do in these cases where two derived state variables come into play at the same time.
	assert(_derived.find(variable) == _derived.end());


	unsigned id = intvars.size();
	intvars << Helper::createPlanningVariable(csp, variable, true);
	
	unsigned reified_id = boolvars.size();
	boolvars << Helper::createBoolVariable(csp);
	
	_derived.insert(std::make_pair(variable, std::make_pair(id, reified_id)));
	return true;
}

bool GecodeCSPVariableTranslator::registerNestedTerm(fs::NestedTerm::cptr nested, CSPVariableType type, SimpleCSP& csp, Gecode::IntVarArgs& variables) {
	TypeIdx domain_type = Problem::getInfo().getFunctionData(nested->getSymbolId()).getCodomainType();
	return registerNestedTerm(nested, type, domain_type, csp, variables);
}

bool GecodeCSPVariableTranslator::registerNestedTerm(fs::NestedTerm::cptr nested, CSPVariableType type, TypeIdx domain_type, SimpleCSP& csp, Gecode::IntVarArgs& variables) {
	TranslationKey key(nested, type);
	auto it = _registered.find(key);
	if (it!= _registered.end()) return false; // The element was already registered

	variables << Helper::createTemporaryVariable(csp, domain_type);

	_registered.insert(it, std::make_pair(key, variables.size()-1));
	return true;
}

bool GecodeCSPVariableTranslator::registerNestedTerm(fs::NestedTerm::cptr nested, CSPVariableType type, int min, int max, SimpleCSP& csp, Gecode::IntVarArgs& variables) {
	TranslationKey key(nested, type);
	auto it = _registered.find(key);
	if (it!= _registered.end()) return false; // The element was already registered

	variables << Helper::createTemporaryIntVariable(csp, min, max);

	_registered.insert(it, std::make_pair(key, variables.size()-1));
	return true;
}

void GecodeCSPVariableTranslator::registerNestedFluent(fs::NestedTerm::cptr term, unsigned table_size, SimpleCSP& csp, Gecode::IntVarArgs& intvars, Gecode::BoolVarArgs& boolvars) {
	TranslationKey key(term, CSPVariableType::Input); // Index variables are always considered input variables
	auto it = element_constraint_data.find(key);
	if (it != element_constraint_data.end()) return; // We already registered an index variable for this particular nested term

	unsigned index_var_idx = intvars.size();
	intvars << Helper::createTemporaryIntVariable(csp, 0, table_size-1);

	unsigned bool_var_idx = boolvars.size();
	// We create one boolean variable for each of the elements of the disjunction IDX=i \lor TABLE_i = DONT_CARE
	for (unsigned i = 0; i < table_size*2; ++i) {
		boolvars << Helper::createBoolVariable(csp);
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
		os << "\t " << info.getVariableName(it.first) << ": " << csp._intvars[it.second.first] << std::endl;
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
	
	Gecode::BoolVarArgs delta_reification_variables;

	
	// Iterate over all the input state variables and constrain them according to the RPG layer
	for (const auto& it:_input_state_variables) {
		VariableIdx variable = it.first;
		unsigned csp_variable_id = it.second.first;
		unsigned reified_variable_id = it.second.second;
		
		const Gecode::IntVar& csp_variable = csp._intvars[csp_variable_id];
		
		Helper::constrainCSPVariable(csp, csp_variable, layer.get_domain(variable));
		
		// Post the "novelty constraint"
		auto& novelty_reification_variable = csp._boolvars[reified_variable_id];
		delta_reification_variables << novelty_reification_variable;
		
		Gecode::dom(csp, csp_variable, layer.get_delta(variable), novelty_reification_variable);
	}

	// Now constrain the derived variables, but not excluding the DONT_CARE value
	for (const auto& it:_derived) {
		VariableIdx variable = it.first;
		unsigned csp_variable_id = it.second.first;
		
		unsigned reified_variable_id = it.second.second;
		
		const Gecode::IntVar& csp_variable = csp._intvars[csp_variable_id];

		Helper::constrainCSPVariable(csp, csp_variable, layer.get_domain(variable), true);
		
		// Post the "novelty constraint"
		auto& novelty_reification_variable = csp._boolvars[reified_variable_id];
		delta_reification_variables << novelty_reification_variable;
		assert(0); // Need to take the DONT_CARE hack into account
		Gecode::dom(csp, csp_variable, layer.get_delta(variable), novelty_reification_variable);
	}
	
	// Now post the global novelty constraint OR: X1 is new, or X2 is new, or...
	Gecode::rel(csp, Gecode::BOT_OR, delta_reification_variables, 1);
}

/*
void GecodeCSPVariableTranslator::postCachingConstraint(SimpleCSP& csp, const GecodeRPGLayer& layer) const {
	


}
*/


PartialAssignment GecodeCSPVariableTranslator::buildAssignment(SimpleCSP& solution) const {
	PartialAssignment assignment;
	for (const auto& it:_input_state_variables) {
		VariableIdx variable = it.first;
		auto& csp_variable = solution._intvars[it.second.first];
		assignment.insert(std::make_pair(variable, csp_variable.val()));
	}
	return assignment;
}

} } // namespaces
