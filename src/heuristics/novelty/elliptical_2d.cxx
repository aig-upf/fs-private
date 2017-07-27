#include <heuristics/novelty/elliptical_2d.hxx>

#include <languages/fstrips/language.hxx>
#include <languages/fstrips/scopes.hxx>
#include <languages/fstrips/operations/basic.hxx>

#include <constraints/soplex/lp.hxx>

#include <problem_info.hxx>
#include <problem.hxx>
#include <heuristics/l2_norm.hxx>
#include <lapkt/tools/logging.hxx>

#include <memory>

namespace fs0 {

    EllipticalMapping2D::EllipticalMapping2D( const std::vector<VariableIdx>& S, std::shared_ptr<State> x0, std::shared_ptr<State> xG)
        : Feature( S, type_id::float_t ), _start(x0), _goal(xG) {

    }

    EllipticalMapping2D::~EllipticalMapping2D() {

    }

    FSFeatureValueT
    EllipticalMapping2D::evaluate( const State& s ) const {
    	return fs0::raw_value<FSFeatureValueT>( make_object( hybrid::L2Norm::measure(scope(), start(), s)
                                                                + hybrid::L2Norm::measure(scope(), s, goal()) ));
    }

    void
    EllipticalMapping2D::make_goal_relative_features( std::vector< lapkt::novelty::NoveltyFeature<State>* >& features ) {
        spx::LinearProgram   poly;
        const Problem& problem = Problem::getInstance();
        std::set<VariableIdx> goal_relevant_vars;
        for ( auto formula : fs::all_relations( *problem.getGoalConditions())) {
            poly.add_constraint(formula);
            std::set<VariableIdx> condS;
            fs::ScopeUtils::computeFullScope(formula, condS);
            for ( auto x : condS ) {
                goal_relevant_vars.insert(x);
            }
		}

        poly.setup();
        poly.solve();

        std::shared_ptr<State> s0 = std::make_shared<State>(problem.getInitialState());
        std::shared_ptr<State> sG = std::make_shared<State>(problem.getInitialState());
        sG->accumulate(poly.solution());

        LPT_INFO("heuristic", "Constructing Ellipitical Mapping features: ");
        const ProblemInfo& info = ProblemInfo::getInstance();


        std::vector<VariableIdx> tmp( goal_relevant_vars.begin(), goal_relevant_vars.end());
        for ( unsigned i = 0; i < tmp.size(); i++ ) {
            for ( unsigned j  = i+1; j < tmp.size(); j ++ ) {
                VariableIdx x0 = tmp[i];
                VariableIdx x1 = tmp[j];
                LPT_INFO( "heuristic", "\t Variables: " << info.getVariableName(x0) << ", " << info.getVariableName(x1));
                std::vector<VariableIdx> input = { x0, x1};
                features.push_back( new EllipticalMapping2D( input, s0, sG ));
            }
        }
    }

    std::ostream&
    EllipticalMapping2D::print(std::ostream& os) const {
        return os;
    }

}
