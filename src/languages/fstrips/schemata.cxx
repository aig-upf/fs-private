
#include <problem_info.hxx>
#include <languages/fstrips/schemata.hxx>
#include <languages/fstrips/language.hxx>
#include <problem.hxx>
#include <utils/utils.hxx>
#include <state.hxx>

namespace fs0 { namespace language { namespace fstrips {


std::ostream& TermSchema::print(std::ostream& os) const { return print(os, Problem::getCurrentProblem()->getProblemInfo()); }

std::ostream& TermSchema::print(std::ostream& os, const fs0::ProblemInfo& info) const { 
	os << "<unnamed unprocessed term>";
	return os;
}

Term* NestedTermSchema::process(const ObjectIdxVector& binding, const ProblemInfo& info) const {
	std::vector<const Term*> st;
	st.reserve(_subterms.size());
	std::vector<ObjectIdx> constant_values;
	for (const TermSchema* unprocessed_subterm:_subterms) {
		const Term* processed = unprocessed_subterm->process(binding, info);
		st.push_back(processed);
		
		if (const Constant* constant = dynamic_cast<const Constant*>(processed)) {
			constant_values.push_back(constant->getValue());
		}
	}
	
	const auto& function = info.getFunctionData(_symbol_id);
	if (function.isStatic()) {
		
		// If all subterms are constants, we can resolve the value of the term schema statically
		if (constant_values.size() == _subterms.size()) { 
			auto value = function.getFunction()(constant_values);
			return new Constant(value);
		}
		
		// Otherwise, we have a statically-headed nested term
		return new StaticHeadedNestedTerm(_symbol_id, st);
	} else {
		// If all subterms were constant, and the symbol is fluent, we have a state variable
		if (constant_values.size() == _subterms.size()) {
			VariableIdx id = info.getVariableId(_symbol_id, constant_values);
			return new StateVariable(id);
		}
		return new FluentHeadedNestedTerm(_symbol_id, st);
	}
}


std::ostream& NestedTermSchema::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	return NestedTerm::printFunction(os, info, _symbol_id, _subterms);
}


Term* ActionSchemaParameter::process(const ObjectIdxVector& binding, const ProblemInfo& info) const {
	assert(_position < binding.size());
	return new Constant(binding.at(_position));
}

std::ostream& ActionSchemaParameter::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "?" << _position;
	return os;
}

Term* ConstantSchema::process(const ObjectIdxVector& binding, const ProblemInfo& info) const {
	return new Constant(_value);
}

std::ostream& ConstantSchema::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << _value;
	return os;
}


AtomicFormulaSchema::cptr AtomicFormulaSchema::create(const std::string& symbol, TermSchema::cptr lhs, TermSchema::cptr rhs) {
	return new AtomicFormulaSchema(AtomicFormula::string_to_symbol.at(symbol), lhs, rhs);
}

AtomicFormula::cptr AtomicFormulaSchema::process(const ObjectIdxVector& binding, const ProblemInfo& info) const {
	return AtomicFormula::create(symbol, lhs->process(binding, info), rhs->process(binding, info));
}

std::ostream& AtomicFormulaSchema::print(std::ostream& os) const { return print(os, Problem::getCurrentProblem()->getProblemInfo()); }

std::ostream& AtomicFormulaSchema::print(std::ostream& os, const fs0::ProblemInfo& info) const { 
	os << *lhs << " " << AtomicFormula::symbol_to_string.at(symbol) << " " << *rhs;
	return os;
}


ActionEffect::cptr ActionEffectSchema::process(const ObjectIdxVector& binding, const ProblemInfo& info) const {
	return new ActionEffect(lhs->process(binding, info), rhs->process(binding, info));
}

std::ostream& ActionEffectSchema::print(std::ostream& os) const { return print(os, Problem::getCurrentProblem()->getProblemInfo()); }

std::ostream& ActionEffectSchema::print(std::ostream& os, const fs0::ProblemInfo& info) const { 
	os << *lhs << " := " << *rhs;
	return os;
}



} } } // namespaces
