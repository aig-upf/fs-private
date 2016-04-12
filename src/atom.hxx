
#pragma once

#include <fs_types.hxx>

namespace fs0 {

//! An atom is a tuple X=x, where X is a state variable and x a value from its domain.
class Atom {
protected:
	//! The state variable
	VariableIdx _variable;

	//! The domain value.
	ObjectIdx  _value;
	
public:
	typedef std::vector<Atom> vctr;
	typedef std::shared_ptr<Atom::vctr> vctrp;

	Atom(VariableIdx variable, ObjectIdx value) : _variable(variable), _value(value) {};
	Atom(const Atom& other) = default;
	Atom(Atom&& other) = default;

	Atom& operator=(const Atom& other) = default;
	Atom& operator=(Atom&& other) = default;

	inline VariableIdx getVariable() const { return _variable; }
	inline ObjectIdx getValue() const { return _value; }

	std::size_t hash() const;
	
	std::ostream& print(std::ostream& os) const;
	friend std::ostream& operator<<(std::ostream &os, const Atom& atom) { return atom.print(os); }
};

//! Comparison operators
inline bool operator==(const Atom& lhs, const Atom& rhs){ return lhs.getVariable() == rhs.getVariable() && lhs.getValue() == rhs.getValue(); }
inline bool operator!=(const Atom& lhs, const Atom& rhs){return !operator==(lhs,rhs);}
inline bool operator< (const Atom& lhs, const Atom& rhs){
	if (lhs.getVariable() < rhs.getVariable()) return true;
	if (lhs.getVariable() > rhs.getVariable()) return false;
	if (lhs.getValue() < rhs.getValue()) return true;
	return false;
}
inline bool operator> (const Atom& lhs, const Atom& rhs){return  operator< (rhs,lhs);}
inline bool operator<=(const Atom& lhs, const Atom& rhs){return !operator> (lhs,rhs);}
inline bool operator>=(const Atom& lhs, const Atom& rhs){return !operator< (lhs,rhs);}

} // namespaces

// Specialization of std::hash for atoms
namespace std {
	template<> struct hash<fs0::Atom> {
		std::size_t operator()(const fs0::Atom& element) const { return element.hash(); }
	};
}