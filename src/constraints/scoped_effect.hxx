
#pragma once

#include <fs0_types.hxx>
#include <state.hxx>
#include <constraints/constraints.hxx>
#include <utils/projections.hxx>

namespace fs0 {

class ScopedEffect
{
protected:
	//! The state variables that make up the scope of the effect.
	const VariableIdxVector _scope;
	
	//! The state variables that are affected by the effect.
	const VariableIdx _affected;
	
	//! A vector of arbitrary parameters
	const std::vector<int> _binding;
	
public:
	typedef ScopedEffect const * cptr;
	typedef std::vector<ScopedEffect::cptr> vcptr;
	
	ScopedEffect(const VariableIdxVector& scope, const VariableIdxVector& image, const std::vector<int>& parameters);
	
	virtual ~ScopedEffect() {}
	
	inline const VariableIdxVector& getScope() const { return _scope; }
	inline VariableIdx getAffected() const { return _affected; }
	inline unsigned getArity() const { return _scope.size(); }
	
	//! To be overriden by the concrete subclasses
	virtual ObjectIdx apply(const ObjectIdxVector& values) const = 0;
	
	//! A small helper
	Fact apply(const State& s) const {
		return Fact(_affected, this->apply(Projections::project(s, _scope)));
	}
};

} // namespaces

