
#include <constraints/direct/effect.hxx>
#include <utils/projections.hxx>

namespace fs0 {

DirectEffect::DirectEffect(const VariableIdxVector& scope, VariableIdx affected, const std::vector<int>& parameters) :
	DirectComponent(scope, parameters), _affected(affected)
{}

ZeroaryDirectEffect::ZeroaryDirectEffect(VariableIdx affected, const std::vector<int>& parameters) :
	DirectEffect({}, affected, parameters)
{}

UnaryDirectEffect::UnaryDirectEffect(VariableIdx relevant, VariableIdx affected, const std::vector<int>& parameters) :
	DirectEffect({relevant}, affected, parameters)
{}

BinaryDirectEffect::BinaryDirectEffect(const VariableIdxVector& scope, VariableIdx affected, const std::vector<int>& parameters) :
	DirectEffect(scope, affected, parameters)
{
	assert(scope.size() == 2);
}

} // namespaces

