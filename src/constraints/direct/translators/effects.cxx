
#include <languages/fstrips/builtin.hxx>
#include <languages/fstrips/operations.hxx>
#include <constraints/direct/translators/effects.hxx>
#include <constraints/direct/builtin.hxx>

namespace fs0 { 
	
	
const DirectEffect* ConstantRhsTranslator::translate(const fs::StateVariable& affected, const fs::Term& rhs) const {
	const auto& constant = dynamic_cast<const fs::Constant&>(rhs);
	return new ValueAssignmentEffect(boost::get<int>(affected.getValue()), boost::get<int>(constant.getValue()));
}

const DirectEffect* StateVariableRhsTranslator::translate(const fs::StateVariable& affected, const fs::Term& rhs) const {
	const auto& variable = dynamic_cast<const fs::StateVariable&>(rhs);
	return new VariableAssignmentEffect(boost::get<int>(variable.getValue()), boost::get<int>(affected.getValue()));
}

const DirectEffect* AdditiveTermRhsTranslator::translate(const fs::StateVariable& affected, const fs::Term& rhs) const {
	const auto& addition = dynamic_cast<const fs::AdditionTerm&>(rhs);
	auto& subterms = addition.getSubterms();
	auto sum_lhs = subterms[0], sum_rhs = subterms[1];
	
	// If some of the subterms is a static expression itself, cannot convert this into an arithmetic DirectEffect.
	if (!fs::flat(*sum_lhs) || !fs::flat(*sum_rhs)) return nullptr; 
	
	auto lhs_var = dynamic_cast<const fs::StateVariable*>(sum_lhs);
	auto rhs_var = dynamic_cast<const fs::StateVariable*>(sum_rhs);
	auto lhs_const = dynamic_cast<const fs::Constant*>(sum_lhs);
	auto rhs_const = dynamic_cast<const fs::Constant*>(sum_rhs);
	
	if (lhs_var && rhs_var)            return new AdditiveBinaryEffect({boost::get<int>(lhs_var->getValue()), boost::get<int>(rhs_var->getValue())}, boost::get<int>(affected.getValue()));
	else if (lhs_var && rhs_const)     return new AdditiveUnaryEffect(boost::get<int>(lhs_var->getValue()), boost::get<int>(affected.getValue()), boost::get<int>(rhs_const->getValue()));
	else if (lhs_const && rhs_var)     return new AdditiveUnaryEffect(boost::get<int>(rhs_var->getValue()), boost::get<int>(affected.getValue()), boost::get<int>(lhs_const->getValue()));
	else throw std::runtime_error("Constant expression should have been compiled earlier");
}

const DirectEffect* SubtractiveTermRhsTranslator::translate(const fs::StateVariable& affected, const fs::Term& rhs) const {
	const auto& addition = dynamic_cast<const fs::SubtractionTerm&>(rhs);
	auto& subterms = addition.getSubterms();
	auto sum_lhs = subterms[0], sum_rhs = subterms[1];
	
	// If some of the subterms is a static expression itself, cannot convert this into an arithmetic DirectEffect.
	if (!fs::flat(*sum_lhs) || !fs::flat(*sum_rhs)) return nullptr; 
	
	auto lhs_var = dynamic_cast<const fs::StateVariable*>(sum_lhs);
	auto rhs_var = dynamic_cast<const fs::StateVariable*>(sum_rhs);
	auto lhs_const = dynamic_cast<const fs::Constant*>(sum_lhs);
	auto rhs_const = dynamic_cast<const fs::Constant*>(sum_rhs);
	
	if (lhs_var && rhs_var)            return new SubtractiveBinaryEffect({boost::get<int>(lhs_var->getValue()), boost::get<int>(rhs_var->getValue())}, boost::get<int>(affected.getValue()));
	else if (lhs_var && rhs_const)     return new SubtractiveUnaryEffect(boost::get<int>(lhs_var->getValue()), boost::get<int>(affected.getValue()), boost::get<int>(rhs_const->getValue()));
	else if (lhs_const && rhs_var)     return new SubtractiveUnaryEffect(boost::get<int>(rhs_var->getValue()), boost::get<int>(affected.getValue()), boost::get<int>(lhs_const->getValue()));
	else throw std::runtime_error("Constant expression should have been compiled earlier");
}

const DirectEffect* MultiplicativeTermRhsTranslator::translate(const fs::StateVariable& affected, const fs::Term& rhs) const {
	const auto& addition = dynamic_cast<const fs::MultiplicationTerm&>(rhs);
	auto& subterms = addition.getSubterms();
	auto sum_lhs = subterms[0], sum_rhs = subterms[1];
	
	// If some of the subterms is a static expression itself, cannot convert this into an arithmetic DirectEffect.
	if (!fs::flat(*sum_lhs) || !fs::flat(*sum_rhs)) return nullptr; 
	
	auto lhs_var = dynamic_cast<const fs::StateVariable*>(sum_lhs);
	auto rhs_var = dynamic_cast<const fs::StateVariable*>(sum_rhs);
	auto lhs_const = dynamic_cast<const fs::Constant*>(sum_lhs);
	auto rhs_const = dynamic_cast<const fs::Constant*>(sum_rhs);
	
	if (lhs_var && rhs_var)            return new MultiplicativeBinaryEffect({boost::get<int>(lhs_var->getValue()), boost::get<int>(rhs_var->getValue())}, boost::get<int>(affected.getValue()));
	else if (lhs_var && rhs_const)     return new MultiplicativeUnaryEffect(boost::get<int>(lhs_var->getValue()), boost::get<int>(affected.getValue()), boost::get<int>(rhs_const->getValue()));
	else if (lhs_const && rhs_var)     return new MultiplicativeUnaryEffect(boost::get<int>(rhs_var->getValue()), boost::get<int>(affected.getValue()), boost::get<int>(lhs_const->getValue()));
	else throw std::runtime_error("Constant expression should have been compiled earlier");
}



} // namespaces
