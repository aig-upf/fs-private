
#include <aptk2/tools/logging.hxx>

#include <search/drivers/sbfws/features/features.hxx>
#include <problem_info.hxx>
#include <heuristics/novelty/features.hxx>
#include <utils/loader.hxx>


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
bool
FeatureSelector<StateT>::has_extra_features() const {
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
	for (VariableIdx var = 0; var < info.getNumVariables(); ++var) {
		features.push_back(new StateVariableFeature(var));
	}
}

void process_feature_list(const rapidjson::Value& data) {
	for (unsigned i = 0; i < data.Size(); ++i) {
		const auto& feat_node = data[i];
		const std::string& feat_name = feat_node["name"].GetString();
		LPT_DEBUG("cout", "Processing feature: " << feat_name);
	}
}


template <typename StateT>
void
FeatureSelector<StateT>::add_extra_features(const ProblemInfo& info, std::vector<FeatureT*>& features) {
	
	try {
		auto data = Loader::loadJSONObject(info.getDataDir() + "/extra.json");
		process_feature_list(data["features"]);
// 		for (auto feature:process_feature_list(data["features"])) {
// 			
// 		}
			
			
			
	} catch(const std::exception& e) {
		return; // No extra.json file could be loaded, therefore we assume there's no extra feature info.
	}
}

// explicit template instantiation
template class FeatureSelector<State>;

} } // namespaces
