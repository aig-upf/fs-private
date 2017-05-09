#include <dynamics/ode.hxx>
#include <problem_info.hxx>

#include <iostream>

namespace fs0 { namespace dynamics {

    std::ostream&
    DifferentialEquation::print(std::ostream& os) const {
        const ProblemInfo& info = ProblemInfo::getInstance();
        os << "delta " << info.getVariableName( _affected ) << " / delta t = \\sum" << std::endl;
        for ( unsigned k = 0; k < _terms.size(); k++ )
            os << "\t" << (_signs[k] < 0.0 ? "-" : "" ) << *_terms[k] << std::endl;

        return os;
    }


}}
