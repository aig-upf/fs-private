#include <fs/core/problem_info.hxx>
#include <fs/core/languages/fstrips/metrics.hxx>
#include <fs/core/problem.hxx>
#include <fs/core/state.hxx>


namespace fs0 { namespace language { namespace fstrips {

    Metric::Metric( const Metric& other )
        : _type(other._type),
        _terminal_expr( other._terminal_expr ? other._terminal_expr->clone() : nullptr ),
        _stage_expr( other._stage_expr ? other._stage_expr->clone() : nullptr ),
        _valid(other._valid) {}

    Metric::~Metric() {
        delete _terminal_expr;
        delete _stage_expr;
    }
    bool Metric::isWellFormed() const {
        return true;
    }

    object_id Metric::evaluate(const State& state, const Term* expr) const {
        if (expr == nullptr)
            throw std::runtime_error("Metric::evaluate(state,expr): expression is nullptr!");
        return expr->interpret(state);
    }

    std::ostream& Metric::print(std::ostream& os) const { return print(os, ProblemInfo::getInstance()); }

    std::ostream& Metric::print(std::ostream& os, const fs0::ProblemInfo& info) const {
        os << (_type == MetricType::MINIMIZE ? "minimize" : "maximize");
        if (terminal_expr() != nullptr ) os << "terminal: " << *terminal_expr() << " ";
        if (stage_expr() != nullptr ) os << "stage: " << *stage_expr() << " ";
    	return os;
    }


} } } // namespaces
