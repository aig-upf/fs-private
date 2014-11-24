
#pragma once

#include <cstdint>
#include <vector>
#include <boost/container/flat_set.hpp>
#include <boost/concept_check.hpp>

#include <core_types.hxx>

namespace aptk { namespace core {

class GenericState;
typedef GenericState State;


class Formula {
public:
	typedef std::shared_ptr<Formula> ptr;
	typedef std::shared_ptr<const Formula> cptr;
	
	//! Keep it virtual
	virtual ~Formula() {};
};

/**
 * A single effect of an action on the state of the world.
 */
class Fact : public Formula
{
public:
	typedef std::shared_ptr<const Fact> cptr;
	
	//! The affected state variable
	VariableIdx _variable; 
	
	//! The new value.
	ObjectIdx  _value;
	
	Fact(const VariableIdx variable, const ObjectIdx value) :
		_variable(variable),
		_value(value)
	{};
	
	//! Keep it virtual
	virtual ~Fact() {};
	
	std::ostream& print(std::ostream& os) const;
	
	virtual const std::string getSign() const { return "="; }
	
	friend std::ostream& operator<<(std::ostream &os, const Fact&  eff) { return eff.print(os); }
	
// 	virtual bool isSatisfiable(const State& state) const;
};

bool operator< (const Fact& lhs, const Fact& rhs);

typedef std::vector<Fact> FactVector;
typedef boost::container::flat_set<Fact> FactSet;
typedef std::shared_ptr<boost::container::flat_set<Fact>> FactSetPtr;



class NegatedFact : public Fact
{
public:
	typedef std::shared_ptr<const NegatedFact> cptr;
	
	NegatedFact(const VariableIdx variable, const ObjectIdx value) :
		Fact(variable, value)
	{};
	
	virtual const std::string getSign() const { return "!="; }
	
	//! Double-dispatch - this is necessary to call the NegatedFact version of ActionManager::checkFormulaSatisfiable
// 	virtual bool isSatisfiable(const State& state) const;
};


// class ConjunctiveFact : Formula
// {
// public:
// 	//! The elements of the conjunction
// 	std::vector<Formula::cptr> elements;
// 	
// 	std::ostream& print(std::ostream& os) const;
// 	
// 	friend std::ostream& operator<<(std::ostream &os, const ConjunctiveFact&  eff) { return eff.print(os); }	
// };


//! Conjunctions of only facts or negated-facts
class PlainAggregatedFact : public Formula
{
public:
	//! The elements of the conjunction
	std::vector<Fact::cptr> elements;
	
	PlainAggregatedFact(const std::vector<Fact::cptr>& facts) :
		elements(facts)
	{}	
	
	std::ostream& print(std::ostream& os) const;
	
	friend std::ostream& operator<<(std::ostream &os, const PlainAggregatedFact&  eff) { return eff.print(os); }
	
	virtual std::string getOperatorName() const = 0;
	
	void add(Fact::cptr fact) {
		elements.push_back(fact);
	}
};

class PlainConjunctiveFact : public PlainAggregatedFact
{
public:
	typedef std::shared_ptr<const PlainConjunctiveFact> cptr;
	
	PlainConjunctiveFact(const std::vector<Fact::cptr>& facts) :
		PlainAggregatedFact(facts)
	{}		

protected:
	std::string getOperatorName() const { return "AND"; }
};

class PlainDisjunctiveFact : public PlainAggregatedFact
{
public:
	typedef std::shared_ptr<const PlainDisjunctiveFact> cptr;
	
	PlainDisjunctiveFact(const std::vector<Fact::cptr>& facts) :
		PlainAggregatedFact(facts)
	{}

protected:
	std::string getOperatorName() const { return "OR"; }
};







} } // namespaces
