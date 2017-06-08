
#include <constraints/direct/effect.hxx>
#include <utils/projections.hxx>
#include <state.hxx>

namespace fs0 {

DirectEffect::DirectEffect(const VariableIdxVector& scope, VariableIdx affected, const ValueTuple& parameters) :
	DirectComponent(scope, parameters), _affected(affected)
{}

Atom DirectEffect::apply(const State& s) const { return this->apply(Projections::project(s, _scope)); }

ZeroaryDirectEffect::ZeroaryDirectEffect(VariableIdx affected, const ValueTuple& parameters) :
	DirectEffect({}, affected, parameters)
{}

UnaryDirectEffect::UnaryDirectEffect(VariableIdx relevant, VariableIdx affected, const ValueTuple& parameters) :
	DirectEffect({relevant}, affected, parameters)
{}

Atom UnaryDirectEffect::apply(const State& s) const { return this->apply(s.getValue(_scope[0])); }

BinaryDirectEffect::BinaryDirectEffect(const VariableIdxVector& scope, VariableIdx affected, const ValueTuple& parameters) :
	DirectEffect(scope, affected, parameters)
{
	assert(scope.size() == 2);
}

Atom BinaryDirectEffect::apply(const State& s) const { return this->apply(s.getValue(_scope[0]), s.getValue(_scope[1])); }

} // namespaces

