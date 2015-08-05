
#pragma once

#include <fs0_types.hxx>

namespace fs0 {


//! A common base class for operative conditions / effects
class DirectComponent {
protected:
	//! The state variables that make up the scope of the component.
	const VariableIdxVector _scope;
	
	//! A vector of arbitrary parameters of the component
	const std::vector<int> _parameters;


public:
	typedef DirectComponent* ptr;
	typedef const DirectComponent* cptr;

	DirectComponent(const VariableIdxVector& scope, const std::vector<int>& parameters) :
		_scope(scope), _parameters(parameters) {}

	virtual ~DirectComponent() {}

	//! Getter for the component scope
	inline const VariableIdxVector& getScope() const { return _scope; }

	//! Return the arity of the component
	inline unsigned getArity () const { return _scope.size(); }
	
	//! Getter for the component binding
	inline const std::vector<int>& getParameters() const { return _parameters; }

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const DirectComponent& o) { return o.print(os); }
	virtual std::ostream& print(std::ostream& os) const {
		os << "<unnamed component>";
		return os;
	}
};
	
} // namespaces
