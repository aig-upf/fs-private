
#pragma once

#include <fs_types.hxx>

namespace fs0 { class Binding; }
namespace fs0 { namespace language { namespace fstrips {

class Formula;
class BoundVariable;

class BindingUnit {
protected:
	//! 'binding_index[i]' contains the index in the binding unit of the i-th parameter of the action / axiom / etc
	std::vector<unsigned> _binding_index;
	
	const std::vector<const BoundVariable*> _variables;
	
	static std::vector<unsigned> index_parameters(const std::vector<std::string>& parameters, const std::vector<const BoundVariable*>& unit);
	
public:
	BindingUnit(const std::vector<std::string>& parameters, const std::vector<const BoundVariable*>& variables) :
		_binding_index(index_parameters(parameters, variables)), _variables(variables)
	{}
	
	~BindingUnit();
	
	BindingUnit(const BindingUnit&);
	
	void update_binding(Binding& binding, const std::vector<ObjectIdx>& values) const;
	
	//! Return the variable that corresponds to parameter 'i'
	const BoundVariable* getParameterVariable(unsigned i) const { return _variables.at(_binding_index.at(i)); }
};

//! All the data that fully characterizes a lifted action
class Axiom {
protected:
	const std::string _name;
	
	const Signature _signature;
	
	//! The IDs of the variables bound by this binding unit
	const std::vector<unsigned> _parameter_ids;
	const std::vector<std::string> _parameter_names;
	BindingUnit _bunit;
	const Formula* _definition;

public:
	Axiom(const std::string& name, const Signature& signature, const std::vector<std::string>& parameters, const BindingUnit& bunit, const Formula* definition);
	~Axiom();
	Axiom(const Axiom&);
	
	const std::string& getName() const { return _name; }
	const Signature& getSignature() const { return _signature; }
	const std::vector<std::string>& getParameterNames() const { return _parameter_names; }
	const BindingUnit& getBindingUnit() const { return _bunit; }
	const Formula* getDefinition() const { return _definition; }
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const Axiom& entity) { return entity.print(os); }
	std::ostream& print(std::ostream& os) const;
};

	
} } } // namespaces
