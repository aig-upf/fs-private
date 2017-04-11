
#include <unordered_map>

#include <languages/fstrips/axioms.hxx>
#include <languages/fstrips/formulae.hxx>
#include <languages/fstrips/terms.hxx>
#include <utils/printers/language.hxx>
#include <utils/binding.hxx>
#include <utils/utils.hxx>


namespace fs0 { namespace language { namespace fstrips {


std::vector<unsigned> BindingUnit::
index_parameters(const std::vector<std::string>& parameters, const std::vector<const BoundVariable*>& unit) {
	std::unordered_map<std::string, unsigned> parameter_ids;
	for (const BoundVariable* var:unit) {
		parameter_ids.insert(std::make_pair(var->getName(), var->getVariableId()));
	}
	
	std::vector<unsigned> index;
	for (const std::string& parameter:parameters) {
		auto it = parameter_ids.find(parameter);
		if (it == parameter_ids.end()) throw std::runtime_error("Unindexed parameter!!");
		index.push_back(it->second);
	}
	return index;
}

BindingUnit::~BindingUnit() {
	for (auto p:_variables) delete p;
}

BindingUnit::BindingUnit(const BindingUnit& other) :
	_binding_index(other._binding_index),
	_variables(Utils::clone(other._variables))
{}

void BindingUnit::
update_binding(Binding& binding, const std::vector<ObjectIdx>& values) const {
	assert(values.size() == _binding_index.size());
	for (unsigned i = 0; i < values.size(); ++i) {
		binding.set(_binding_index[i], values[i]);
	}
}


Axiom::Axiom(const std::string& name, const Signature& signature, const std::vector<std::string>& parameters, const BindingUnit& bunit, const Formula* definition)
	: _name(name), _signature(signature), _parameter_names(parameters), _bunit(bunit), _definition(definition)
{}


Axiom::~Axiom() {
	delete _definition;
}

Axiom::Axiom(const Axiom& other) :
	_name(other._name),
	_signature(other._signature),
	_parameter_names(other._parameter_names),
	_bunit(other._bunit),
	_definition(other._definition->clone())
{}

std::ostream& Axiom::print(std::ostream& os) const { 
	os <<  print::axiom_header(*this);
	return os;
}

} } } // namespaces
