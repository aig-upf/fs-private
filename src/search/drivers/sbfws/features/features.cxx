
#include <lapkt/tools/logging.hxx>

#include <search/drivers/sbfws/features/features.hxx>
#include <problem_info.hxx>
#include <heuristics/novelty/features.hxx>
#include <heuristics/novelty/squared_error.hxx>
#include <heuristics/novelty/triangle_inequality.hxx>
#include <heuristics/novelty/elliptical_2d.hxx>
#include <utils/loader.hxx>
#include <utils/printers/binding.hxx>
#include <utils/binding_iterator.hxx>
#include <utils/config.hxx>
#include <constraints/registry.hxx>
#include <languages/fstrips/language.hxx>
#include <languages/fstrips/operations/basic.hxx>
#include <problem.hxx>
#include <actions/actions.hxx>
#include <heuristics/l0.hxx>
#include <heuristics/l2_norm.hxx>

#include <lapkt/tools/logging.hxx>


namespace fs = fs0::language::fstrips;

namespace fs0 { namespace bfws {

template <typename StateT>
typename FeatureSelector<StateT>::EvaluatorT
FeatureSelector<StateT>::select() {

	std::vector<FeatureT*> features;
	add_state_variables(_info, features);

	add_extra_features(_info, features);

	// Dump all features into an evaluator and return it
	EvaluatorT evaluator;
	for (auto f:features) {
		evaluator.add(f);
	}
	return evaluator;
}

template <typename StateT>
void
FeatureSelector<StateT>::select(FeatureSelector<StateT>::EvaluatorT& evaluator) {

	std::vector<FeatureT*> features;
	add_state_variables(_info, features);

	add_extra_features(_info, features);

	// Dump all features into an evaluator and return it
	for (auto f:features) {
		evaluator.add(f);
	}
}

template <typename StateT>
bool
FeatureSelector<StateT>::has_extra_features() const {

	// TODO This is a hack, should use a specially named feature in extra.json perhaps¿??
	if (Config::instance().getOption<bool>("use_precondition_counts", false)) return true;

    	if (Config::instance().getOption<bool>("features.l0_sets", false)) return true;

	// TODO - This is repeating work that is done afterwards when loading the features - can be optimized
	try {
		auto data = Loader::loadJSONObject(_info.getDataDir() + "/extra.json");
		return data["features"].Size() > 0;
	} catch(const std::exception& e) {
		return false;
	}
}

template <typename StateT>
void
FeatureSelector<StateT>::add_state_variables(const ProblemInfo& info, std::vector<FeatureT*>& features) {

    std::set<VariableIdx> projected_away;
    if ( Config::instance().getOption<bool>("features.project_away_time",false) ) {
        VariableIdx clock = info.getVariableId("clock_time()");
        projected_away.insert( clock );
        LPT_INFO("main", "Projecting away variable: " << info.getVariableName(clock)  );
    }

    if ( Config::instance().getOption<bool>("features.project_away_numeric",false) ) {
        for ( VariableIdx var = 0; var < info.getNumVariables(); ++var ) {
            if ( info.sv_type(var) ==  type_id::float_t ) {
                projected_away.insert(var);
                LPT_INFO("main", "Projecting away variable: " << info.getVariableName(var)  );
            }
        }
    }

	for (VariableIdx var = 0; var < info.getNumVariables(); ++var) {
        if ( projected_away.find(var) != projected_away.end() )
            continue;
		features.push_back(new StateVariableFeature(var));
	}
}

lapkt::novelty::NoveltyFeature<State>* generate_arbitrary_feature(const ProblemInfo& info,  const std::string& feat_name, const std::vector<object_id>& parameters) {
	unsigned symbol_id = info.getSymbolId(feat_name);

	std::vector<const fs::Term*> subterms;
	for (const object_id&
 value:parameters) {
		subterms.push_back(new fs::Constant(value, UNSPECIFIED_NUMERIC_TYPE));
	}

	if (info.isPredicate(symbol_id)) {
		auto formula =  LogicalComponentRegistry::instance().instantiate_formula(feat_name, subterms);
		return new ArbitraryFormulaFeature(formula);

	} else {
		auto term =  LogicalComponentRegistry::instance().instantiate_term(feat_name, subterms);
		return new ArbitraryTermFeature(term);
	}
}

void process_precondition_count(const ProblemInfo& info, std::vector<lapkt::novelty::NoveltyFeature<State>*>& features) {
	const auto& actions = Problem::getInstance().getGroundActions();

	for (const GroundAction* action:actions) {
		auto precondition = dynamic_cast<const fs::Conjunction*>(action->getPrecondition());
		if (!precondition) throw std::runtime_error("Cannot use precondition-counts as a feature for non-conjunctive preconditions");
		ConditionSetFeature* feature = new ConditionSetFeature();
		for (const auto atom:precondition->getSubformulae()) feature->addCondition(atom);

		features.push_back(feature);
	}

	LPT_INFO("cout", "Added " << actions.size() << " precondition-count features");
}

void process_feature(const ProblemInfo& info, const std::string& feat_name, std::vector<lapkt::novelty::NoveltyFeature<State>*>& features) {
	//! Some specially-named features receive a distinct treatment
// 	if (feat_name == "precondition_count") return process_precondition_count(info, features);


	// Otherwise, we apply thestandard treatment
	unsigned symbol_id = info.getSymbolId(feat_name);
	const SymbolData& sdata = info.getSymbolData(symbol_id);
	const Signature& signature = sdata.getSignature();

	LPT_DEBUG("cout", "Processing feature: " << feat_name << ", with signature: (" << print::raw_signature(signature) << ")");

	// Arity-0 feature, dealt with separately:
	if (signature.empty()) {
		auto feature = generate_arbitrary_feature(info, feat_name, Binding::EMPTY_BINDING.get_full_binding());
		LPT_DEBUG("cout", "Generated feature: " << *feature);
		features.push_back(feature);
		return;
	}

	for (utils::binding_iterator binding_generator(signature, info); !binding_generator.ended(); ++binding_generator) {
		auto feature = generate_arbitrary_feature(info, feat_name, (*binding_generator).get_full_binding());
		LPT_DEBUG("cout", "Generated feature: " << *feature);
		features.push_back(feature);
	}
}


template <typename StateT>
void
FeatureSelector<StateT>::add_extra_features(const ProblemInfo& info, std::vector<FeatureT*>& features) {

	if (Config::instance().getOption<bool>("use_precondition_counts", false)) {
		process_precondition_count(info, features);
	}

	if (Config::instance().getOption<bool>("features.l0_sets", false)) {
		std::shared_ptr<L0Heuristic> l0_extractor = std::make_shared<L0Heuristic>(Problem::getInstance());
		for ( auto formula : l0_extractor->relational() ) {
			ConditionSetFeature* feature = new ConditionSetFeature;
			feature->addCondition(formula);
			features.push_back( feature );
		}
	}

	if (Config::instance().getOption<bool>("features.joint_goal_error", false)) {
		hybrid::L2Norm norm;
		SquaredErrorFeature* feature = new SquaredErrorFeature;
		for ( auto formula : fs::all_formulae( *Problem::getInstance().getGoalConditions())) {
			feature->addCondition(formula);
			norm.add_condition(formula);
		}
		LPT_INFO("features", "Added 'joint_goal_error': phi(s0) = " << feature->error_signal().measure(Problem::getInstance().getInitialState()));
		LPT_INFO("features", "L^2 norm(s0,sG) =  " << norm.measure(Problem::getInstance().getInitialState()));
		features.push_back( feature );
	}

	if (Config::instance().getOption<bool>("features.independent_goal_error", false)) {
		hybrid::L2Norm norm;
		for ( auto formula : fs::all_relations( *Problem::getInstance().getGoalConditions())) {
			SquaredErrorFeature* feature = new SquaredErrorFeature;
			feature->addCondition(formula);
			norm.add_condition( formula );
			features.push_back( feature );
			LPT_INFO("features", "Added 'independent_goal_error': formula: " << *formula << " phi(s0) = " << feature->error_signal().measure(Problem::getInstance().getInitialState()));
		}
		LPT_INFO("features", "L^2 norm(s0,sG) =  " << norm.measure(Problem::getInstance().getInitialState()));
	}

	if ( Config::instance().getOption<bool>("features.triangle_inequality_goal", false)) {
		hybrid::L2Norm norm;
		TriangleInequality* feature = new TriangleInequality;
		for ( auto formula : fs::all_relations( *Problem::getInstance().getGoalConditions())) {
			feature->addCondition(formula);
			norm.add_condition(formula);
		}
		LPT_INFO("features", "Added 'triangle_inequality_goal': phi(s0) = " << feature->norm().measure(Problem::getInstance().getInitialState()));
		LPT_INFO("features", "L^2 norm(s0,sG) =  " << norm.measure(Problem::getInstance().getInitialState()));
		features.push_back( feature );
	}

	if ( Config::instance().getOption<bool>("features.elliptical_2d", false)) {
		EllipticalMapping2D::make_goal_relative_features( features );
        EllipticalMapping2D::print_to_JSON( "elliptical_2d.features.json", features );
	}

	try {
		auto data = Loader::loadJSONObject(info.getDataDir() + "/extra.json");
		const auto& features_node = data["features"];
		for (unsigned i = 0; i < features_node.Size(); ++i) {
			const auto& feature_node = features_node[i];
			process_feature(info, feature_node["name"].GetString(), features);
		}

	} catch(const std::exception& e) {
		return; // No extra.json file could be loaded, therefore we assume there's no extra feature info.
	}

}

// explicit template instantiation
template class FeatureSelector<State>;

} } // namespaces
