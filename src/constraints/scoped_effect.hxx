
#pragma once

#include <fs0_types.hxx>
#include <state.hxx>
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
	
	
	virtual ObjectIdx apply() const {
		throw std::runtime_error("This method can only be used by 0-ary effects");
	};
	
	virtual ObjectIdx apply(ObjectIdx value) const {
		throw std::runtime_error("This method can only be used by unary effects");
	};
	
	virtual ObjectIdx apply(ObjectIdx v1, ObjectIdx v2) const {
		throw std::runtime_error("This method can only be used by binary effects");
	};
	
	//! A small helper
	inline virtual Fact apply(const State& s) const { return Fact(_affected, this->apply(Projections::project(s, _scope))); }
};

//! We specialize this class for performance reasons, since it is so common.
class ZeroaryScopedEffect : public ScopedEffect
{
public:
	ZeroaryScopedEffect(const VariableIdxVector& scope, const VariableIdxVector& image, const std::vector<int>& parameters);
	virtual ~ZeroaryScopedEffect() {}
	
	//! To be overriden by the concrete subclasses
	ObjectIdx apply(const ObjectIdxVector& values) const {
		throw std::runtime_error("0-ary effects are expected not to use this method");
	};
	
	//! To be overriden by concrete subclasses
	ObjectIdx apply() const = 0;
	
	//! A small helper
	inline Fact apply(const State& s) const { return Fact(_affected, this->apply()); }
};

//! We specialize this class for performance reasons, since it is so common.
class UnaryScopedEffect : public ScopedEffect
{
public:
	UnaryScopedEffect(const VariableIdxVector& scope, const VariableIdxVector& image, const std::vector<int>& parameters);
	virtual ~UnaryScopedEffect() {}
	
	//! To be overriden by the concrete subclasses
	ObjectIdx apply(const ObjectIdxVector& values) const {
		throw std::runtime_error("Unary effects are expected not to use this method");
	};
	
	//! To be overriden by concrete subclasses
	ObjectIdx apply(ObjectIdx value) const = 0;
	
	//! A small helper
	Fact apply(const State& s) const { return Fact(_affected, this->apply(s.getValue(_scope[0]))); }
};

//! We specialize this class for performance reasons, since it is so common.
class BinaryScopedEffect : public ScopedEffect
{
public:
	BinaryScopedEffect(const VariableIdxVector& scope, const VariableIdxVector& image, const std::vector<int>& parameters);
	virtual ~BinaryScopedEffect() {}
	
	//! To be overriden by the concrete subclasses
	ObjectIdx apply(const ObjectIdxVector& values) const {
		throw std::runtime_error("Binary effects are expected not to use this method");
	};
	
	//! To be overriden by concrete subclasses
	ObjectIdx apply(ObjectIdx v1, ObjectIdx v2) const = 0;
	
	//! A small helper
	Fact apply(const State& s) const { return Fact(_affected, this->apply(s.getValue(_scope[0]), s.getValue(_scope[1]))); }
};

} // namespaces

