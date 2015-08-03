
#pragma once
#include <fs0_types.hxx>
#include <languages/fstrips/language.hxx>


namespace fs0 { class State; class ProblemInfo; }

namespace fs0 { namespace language { namespace fstrips {

//! A term that has not yet been processed, meaning that it might possibly contain references to yet-unresolved values of action parameters,
//! non-consolidated state variables, etc.
class TermSchema {
public:
	typedef const TermSchema* cptr;
	
	TermSchema() {}
	virtual ~TermSchema() {}

	virtual TermSchema* clone() const = 0;
	
	//! Processes a possibly nested unprocessed term, consolidating the existing state variables
	//! and binding action parameters to concrete language constants.
	virtual Term::cptr process(const ObjectIdxVector& binding, const ProblemInfo& info) const = 0;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const TermSchema& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	virtual std::ostream& print(std::ostream& os, const ProblemInfo& info) const;
};


class NestedTermSchema : public TermSchema {
public:
	NestedTermSchema(unsigned symbol_id, const std::vector<TermSchema::cptr>& subterms)
		: _symbol_id(symbol_id), _subterms(subterms)
	{}
	
	virtual ~NestedTermSchema() {
		for (TermSchema::cptr term:_subterms) delete term;
	}
	
	NestedTermSchema(const NestedTermSchema& term)
		: _symbol_id(term._symbol_id) {
		for (TermSchema::cptr subterm:term._subterms) {
			_subterms.push_back(subterm->clone());
		}
	}
	
	NestedTermSchema* clone() const { return new NestedTermSchema(*this); }
	
	//! Processes a possibly nested unprocessed term, consolidating the existing state variables
	//! and binding action parameters to concrete language constants.
	Term::cptr process(const ObjectIdxVector& binding, const ProblemInfo& info) const;
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const ProblemInfo& info) const;
	
protected:
	//! The ID of the function or predicate symbol, e.g. in the state variable loc(A), the id of 'loc'
	unsigned _symbol_id;
	
	//! The tuple of fixed, constant symbols of the state variable, e.g. {A, B} in the state variable 'on(A,B)'
	std::vector<TermSchema::cptr> _subterms;
};

class BuiltinNestedTermSchema : public TermSchema {
public:
	BuiltinNestedTermSchema(const std::string& symbol, const std::vector<TermSchema::cptr>& subterms)
		: _symbol(symbol), _subterms(subterms)
	{
		assert(subterms.size() == 2);
	}
	
	virtual ~BuiltinNestedTermSchema() {
		for (TermSchema::cptr term:_subterms) delete term;
	}
	
	BuiltinNestedTermSchema(const BuiltinNestedTermSchema& term)
		: _symbol(term._symbol) {
		for (TermSchema::cptr subterm:term._subterms) {
			_subterms.push_back(subterm->clone());
		}
	}
	
	BuiltinNestedTermSchema* clone() const { return new BuiltinNestedTermSchema(*this); }
	
	//! Processes a possibly nested unprocessed term, consolidating the existing state variables
	//! and binding action parameters to concrete language constants.
	Term::cptr process(const ObjectIdxVector& binding, const ProblemInfo& info) const;
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const ProblemInfo& info) const;
	
protected:
	//! The ID of the function or predicate symbol, e.g. in the state variable loc(A), the id of 'loc'
	std::string _symbol;
	
	//! The tuple of fixed, constant symbols of the state variable, e.g. {A, B} in the state variable 'on(A,B)'
	std::vector<TermSchema::cptr> _subterms;
};


//! A state variable whose value depends on a certain binding of values to parameters, e.g. parameters in an action schema.
class ActionSchemaParameter : public TermSchema {
public:
	ActionSchemaParameter(unsigned position) : _position(position) {}
	
	ActionSchemaParameter* clone() const { return new ActionSchemaParameter(*this); }
	
	//! Processes a possibly nested unprocessed term, consolidating the existing state variables
	//! and binding action parameters to concrete language constants.
	Term::cptr process(const ObjectIdxVector& binding, const ProblemInfo& info) const;
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const ProblemInfo& info) const;
	
protected:
	//! The position of the parameter within the ordered set of action parameters
	unsigned _position;
};

//! A simple constant
class ConstantSchema : public TermSchema {
public:
	ConstantSchema(ObjectIdx value)  : _value(value) {}
	
	ConstantSchema* clone() const { return new ConstantSchema(*this); }
	
	//! Processes a possibly nested unprocessed term, consolidating the existing state variables
	//! and binding action parameters to concrete language constants.
	Term::cptr process(const ObjectIdxVector& binding, const ProblemInfo& info) const;

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const ProblemInfo& info) const;
	
protected:
	//! The actual value of the constant
	ObjectIdx _value;
};


class AtomicFormulaSchema {
public:
	typedef const AtomicFormulaSchema* cptr;

	AtomicFormulaSchema(const std::string& symbol ,const std::vector<TermSchema::cptr>& subterms) : _symbol(symbol), _subterms(subterms) {}	
	
	virtual ~AtomicFormulaSchema() { 
		for (const auto ptr:_subterms) delete ptr;
	}
	
	virtual AtomicFormula::cptr process(const ObjectIdxVector& binding, const ProblemInfo& info) const;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const AtomicFormulaSchema& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
	
protected:
	//! The symbol identifying the external method
	const std::string _symbol;
	
	std::vector<TermSchema::cptr> _subterms;
};


class ActionEffectSchema {
public:
	typedef const ActionEffectSchema* cptr;
	
	ActionEffectSchema(TermSchema::cptr lhs_, TermSchema::cptr rhs_) : lhs(lhs_), rhs(rhs_) {}
	
	virtual ~ActionEffectSchema() {
		delete lhs; delete rhs;
	}
	
	ActionEffect::cptr process(const ObjectIdxVector& binding, const ProblemInfo& info) const;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ActionEffectSchema& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
	
	TermSchema::cptr lhs;
	TermSchema::cptr rhs;
protected:
};

} } } // namespaces
