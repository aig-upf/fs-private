
#pragma once

#include <fs0_types.hxx>
#include <state.hxx>
#include <utils/projections.hxx>

namespace fs0 {

/**
 * A generic action effect.
 */
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
	
	//! The generic appllicable / apply methods, to be overriden by concrete subclasses.
	//! In the case of applicable, it is only of use for conditional effects.
	virtual bool applicable(const ObjectIdxVector& values) const { return true; };
	virtual Atom apply(const ObjectIdxVector& values) const = 0;
	
	//! The 6 methods below are specializations for 0-ary / unary / binary effects that exist only for performance reasons. A bit hackish, but works.
	virtual bool applicable() const { throw std::runtime_error("This method can only be used by 0-ary effects"); };
	virtual Atom apply() const { throw std::runtime_error("This method can only be used by 0-ary effects"); }
	
	virtual bool applicable(ObjectIdx value) const { throw std::runtime_error("This method can only be used by unary effects"); };
	virtual Atom apply(ObjectIdx value) const { throw std::runtime_error("This method can only be used by unary effects"); }
	
	virtual bool applicable(ObjectIdx v1, ObjectIdx v2) const { throw std::runtime_error("This method can only be used by binary effects"); };
	virtual Atom apply(ObjectIdx v1, ObjectIdx v2) const { throw std::runtime_error("This method can only be used by binary effects"); }
	
	//! A small helper
	inline virtual Atom apply(const State& s) const { return this->apply(Projections::project(s, _scope)); }
};

//! We specialize this class for performance reasons, since it is so common.
class ZeroaryScopedEffect : public ScopedEffect
{
public:
	ZeroaryScopedEffect(const VariableIdxVector& scope, const VariableIdxVector& image, const std::vector<int>& parameters);
	virtual ~ZeroaryScopedEffect() {}
	
	//! Effects are by default applicable, i.e. condition-less.
	virtual bool applicable() const { return true; };
	
	//! 0-ary effects should use the specialized version
	Atom apply(const ObjectIdxVector& values) const { throw std::runtime_error("0-ary effects are expected not to use this method"); };
	
	//! To be overriden by the concrete effect.
	virtual Atom apply() const = 0;
	
	//! A small helper
	inline Atom apply(const State& s) const { return this->apply(); }
};

//! We specialize this class for performance reasons, since it is so common.
class UnaryScopedEffect : public ScopedEffect
{
public:
	UnaryScopedEffect(const VariableIdxVector& scope, const VariableIdxVector& image, const std::vector<int>& parameters);
	virtual ~UnaryScopedEffect() {}
	
	//! Effects are by default applicable, i.e. condition-less.
	virtual bool applicable(ObjectIdx value) const { return true; };
	
	//! Unary effects should use the specialized version
	Atom apply(const ObjectIdxVector& values) const { throw std::runtime_error("Unary effects are expected not to use this method"); };
	
	//! To be overriden by the concrete effect.
	virtual Atom apply(ObjectIdx value) const = 0;
	
	//! A small helper
	Atom apply(const State& s) const { return this->apply(s.getValue(_scope[0])); }
};

//! We specialize this class for performance reasons, since it is so common.
class BinaryScopedEffect : public ScopedEffect
{
public:
	BinaryScopedEffect(const VariableIdxVector& scope, const VariableIdxVector& image, const std::vector<int>& parameters);
	virtual ~BinaryScopedEffect() {}
	
	//! Effects are by default applicable, i.e. condition-less.
	virtual bool applicable(ObjectIdx v1, ObjectIdx v2) const { return true; };
		
	//! This might be necessary in some cases.
	Atom apply(const ObjectIdxVector& values) const {
		assert(values.size() == 2);
		return apply(values[0], values[1]);
	};
	
	//! To be overriden by the concrete effect.
	virtual Atom apply(ObjectIdx v1, ObjectIdx v2) const = 0;
	
	//! A small helper
	Atom apply(const State& s) const { return this->apply(s.getValue(_scope[0]), s.getValue(_scope[1])); }
};

} // namespaces

