

#pragma once
#include <fs/core/fs_types.hxx>

namespace fs0 { namespace lang { namespace fstrips {

//! The ID of a type (aka sort)
using TypeId = unsigned;

//! The ID of a predicate or function symbol
using SymbolId = unsigned;

//!
using DomainPoint = std::vector<object_id>;

//!
using Signature = std::vector<TypeId>;

//! A signature with variable names
class NamedSignature {
public:
	NamedSignature(const Signature& types, const std::vector<std::string>& names);
};

//! The type of a logical symbol, function or predicate
enum class SymbolType {
	Function,
	Predicate
};

//! The supported numeric types
enum class NumericType {
	Int,
	Real
};


class UndefinedPoint : public std::runtime_error {
public:
        UndefinedPoint(const std::string& msg) : std::runtime_error(msg) {}
};



//!
class InterpretationI {
public:
	virtual bool defined(SymbolId symbol, DomainPoint) = 0;
	virtual object_id value(SymbolId symbol, DomainPoint) = 0;
};

//! 
class FixedInterpretation : public InterpretationI {
protected:
public:
	bool defined(SymbolId symbol, DomainPoint) override;
	object_id value(SymbolId symbol, DomainPoint) override;
};

//! 
class StateInterpretation : public InterpretationI {
protected:
public:
	bool defined(SymbolId symbol, DomainPoint) override;
	object_id value(SymbolId symbol, DomainPoint) override;
};

//! 
class PartialInterpretation : public InterpretationI {
protected:
public:
	bool defined(SymbolId symbol, DomainPoint) override;
	object_id value(SymbolId symbol, DomainPoint) override;
};


class Factory {
public:
	
	//!
	TypeId create_type(const std::string& name);
	
	//!
	TypeId create_numeric_type(const std::string& name, int min, int max);
	
	//!
	TypeId create_numeric_type(const std::string& name, float min, float max);
	
	//! Create a logical symbol (either function or predicate) with given name and signature.
	SymbolId create_symbol(const std::string& name, const NamedSignature& signature, SymbolType type);
	
	//! Create a logical variable with given name.
	unsigned create_variable(const std::string& name, const TypeId& type);
	
	
protected:
	
};	

	
} } } // namespaces
