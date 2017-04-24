#include <problem_info.hxx>
#include <languages/fstrips/metrics.hxx>
#include <problem.hxx>
#include <state.hxx>


namespace fs0 { namespace language { namespace fstrips {


std::vector<Term::cptr> Metric::flatten() const {
    return _expression->flatten();
}

bool Metric::isWellFormed() const {
    return true;
}

ObjectIdx Metric::apply(const State& state) const {
    return _expression->interpret(state);
}

std::ostream& Metric::print(std::ostream& os) const { return print(os, Problem::getInfo()); }

std::ostream& Metric::print(std::ostream& os, const fs0::ProblemInfo& info) const {
    os << (_type == MetricType::MINIMIZE ? "minimize" : "maximize") << " " << *_expression;
	return os;
}


} } } // namespaces
