

#pragma once

#include <languages/fstrips/terms.hxx>
#include <constraints/gecode/rpg_layer.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <boost/functional/hash.hpp>
#include <gecode/int.hh>
#include <languages/fstrips/language.hxx>
#include <languages/fstrips/scopes.hxx>

namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

class VariableCounter;

class NestedFluentElementTranslator {
public:
	
	NestedFluentElementTranslator(fs::FluentHeadedNestedTerm::cptr term, const VariableCounter& counter);
	
	void register_variables(CSPVariableType type, GecodeCSPVariableTranslator& translator);
	
	void register_constraints(CSPVariableType type, GecodeCSPVariableTranslator& translator);
	
	fs::FluentHeadedNestedTerm::cptr getTerm() const { return _term; }
	
	const NestedFluentData& getNestedFluentData() const { return _fluent_data; }

protected:
	
	fs::FluentHeadedNestedTerm::cptr _term;
	
	const VariableCounter& _counter;
	
	NestedFluentData _fluent_data;

	//! Register the constraints related to the don't care optimization
	void register_dont_care_constraints(GecodeCSPVariableTranslator& translator);
};


} } // namespaces

