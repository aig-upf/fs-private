
#include <constraints/scoped_effect.hxx>
#include <utils/projections.hxx>

namespace fs0 {

ScopedEffect::ScopedEffect(const VariableIdxVector& scope, VariableIdx affected, const std::vector<int>& parameters) :
	_scope(scope), _affected(affected), _binding(parameters)
{
}

ZeroaryScopedEffect::ZeroaryScopedEffect(const VariableIdxVector& scope, VariableIdx affected, const std::vector<int>& parameters) :
	ScopedEffect(scope, affected, parameters)
{
	assert(scope.size() == 0);
}

UnaryScopedEffect::UnaryScopedEffect(const VariableIdxVector& scope, VariableIdx affected, const std::vector<int>& parameters) :
	ScopedEffect(scope, affected, parameters)
{
	assert(scope.size() == 1);
}

BinaryScopedEffect::BinaryScopedEffect(const VariableIdxVector& scope, VariableIdx affected, const std::vector<int>& parameters) :
	ScopedEffect(scope, affected, parameters)
{
	assert(scope.size() == 2);
}

} // namespaces

