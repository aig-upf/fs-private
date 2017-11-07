
#pragma once

#include <utils/config.hxx>

#ifdef FS_HYBRID
	#include <modules/hybrid/heuristics/l2_norm.hxx>
	#include <modules/hybrid/heuristics/elliptical_2d.hxx>
#endif


namespace fs0 {

class HybridNoveltyFeaturesWrapper {
public:
	template <typename FeatureT>
	static void register_features(std::vector<FeatureT*>& features) {
		if (Config::instance().getOption<bool>("features.elliptical_2d", false)) {
#ifdef FS_HYBRID
			EllipticalMapping2D::make_goal_relative_features(features);
			EllipticalMapping2D::print_to_JSON("elliptical_2d.features.json", features);			
#else
			throw std::runtime_error("You need to compile the planner with soplex support in order to use \"features.elliptical_2d\" option");
#endif			
		}
		
		
		if (Config::instance().getOption<bool>("features.independent_goal_error", false)) {
#ifdef FS_HYBRID
			hybrid::L2Norm norm;
			for ( auto formula : fs::all_relations( *Problem::getInstance().getGoalConditions())) {
				SquaredErrorFeature* feature = new SquaredErrorFeature;
				feature->addCondition(formula);
				norm.add_condition( formula );
				features.push_back( feature );
				LPT_INFO("features", "Added 'independent_goal_error': formula: " << *formula << " phi(s0) = " << feature->error_signal().measure(Problem::getInstance().getInitialState()));
			}
			LPT_INFO("features", "L^2 norm(s0,sG) =  " << norm.measure(Problem::getInstance().getInitialState()));
			
#else
			throw std::runtime_error("You need to compile the planner with soplex support in order to use \"features.independent_goal_error\" option");
#endif
		}

		if ( Config::instance().getOption<bool>("features.triangle_inequality_goal", false)) {
#ifdef FS_HYBRID
			hybrid::L2Norm norm;
			TriangleInequality* feature = new TriangleInequality;
			for ( auto formula : fs::all_relations( *Problem::getInstance().getGoalConditions())) {
				feature->addCondition(formula);
				norm.add_condition(formula);
			}
			LPT_INFO("features", "Added 'triangle_inequality_goal': phi(s0) = " << feature->norm().measure(Problem::getInstance().getInitialState()));
			LPT_INFO("features", "L^2 norm(s0,sG) =  " << norm.measure(Problem::getInstance().getInitialState()));
			features.push_back( feature );
			
#else
			throw std::runtime_error("You need to compile the planner with soplex support in order to use \"features.triangle_inequality_goal\" option");
#endif			
		}		
		
		
		
		
	}
};

} // namespaces
