#include <fs/core/problem_info.hxx>
#include <fs/core/languages/fstrips/metrics.hxx>
#include <fs/core/problem.hxx>
#include <fs/core/state.hxx>


namespace fs0 { namespace language { namespace fstrips {

Metric::Metric( const Metric& other )
    : _type(other._type),
    _expression( other._expression->clone() ),
    _valid(other._valid) {}

bool Metric::isWellFormed() const {
    return true;
}

object_id Metric::apply(const State& state) const {
    return _expression->interpret(state);
}

std::ostream& Metric::print(std::ostream& os) const { return print(os, ProblemInfo::getInstance()); }

std::ostream& Metric::print(std::ostream& os, const fs0::ProblemInfo& info) const {
    os << (_type == MetricType::MINIMIZE ? "minimize" : "maximize") << " " << *_expression;
	return os;
}


} } } // namespaces
