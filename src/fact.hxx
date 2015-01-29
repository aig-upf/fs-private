
#pragma once

#include <cstdint>
#include <vector>
#include <boost/container/flat_set.hpp>

#include <fs0_types.hxx>

namespace fs0 {

/**
 *  A fact (i.e. an atom) is a tuple X=x, where X is a state variable and x a value from its domain.
 */
class Fact // : public Formula
{
public:
	typedef std::vector<Fact> vctr;
	typedef std::shared_ptr<Fact::vctr> vctrp;
	
	Fact(const VariableIdx variable, const ObjectIdx value);
	
	inline VariableIdx getVariable() const { return _variable; }
	inline ObjectIdx getValue() const { return _value; }
	
	std::ostream& print(std::ostream& os) const;
	friend std::ostream& operator<<(std::ostream &os, const Fact&  eff) { return eff.print(os); }
	virtual const std::string getSign() const { return "="; }

protected:
	//! The state variable
	VariableIdx _variable; 
	
	//! The domain value.
	ObjectIdx  _value;
};

//! Comparison operators
inline bool operator==(const Fact& lhs, const Fact& rhs){ return lhs.getVariable() == rhs.getVariable() && lhs.getValue() == rhs.getValue(); }
inline bool operator!=(const Fact& lhs, const Fact& rhs){return !operator==(lhs,rhs);}
inline bool operator< (const Fact& lhs, const Fact& rhs){ 
	if (lhs.getVariable() < rhs.getVariable()) return true;
	if (lhs.getVariable() > rhs.getVariable()) return false;
	if (lhs.getValue() < rhs.getValue()) return true;
	return false;
}
inline bool operator> (const Fact& lhs, const Fact& rhs){return  operator< (rhs,lhs);}
inline bool operator<=(const Fact& lhs, const Fact& rhs){return !operator> (lhs,rhs);}
inline bool operator>=(const Fact& lhs, const Fact& rhs){return !operator< (lhs,rhs);}


} // namespaces
