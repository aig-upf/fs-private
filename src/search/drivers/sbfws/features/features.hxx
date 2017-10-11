
#pragma once
#include <lapkt/novelty/features.hxx>

#include <state.hxx>

namespace fs0 { class ProblemInfo; }

namespace fs0 { namespace bfws {

class FeatureIndex;
	
//!
template <typename StateT>
class FeatureSelector {
public:
	using EvaluatorT = lapkt::novelty::GenericFeatureSetEvaluator<StateT>;
	using FeatureT = typename EvaluatorT::FeatureT;
	
	FeatureSelector(const ProblemInfo& info) : _info(info) {}
	
	bool has_extra_features() const;
	
	EvaluatorT select(FeatureIndex* featidx = nullptr);
	
	void add_state_variables(const ProblemInfo& info, std::vector<FeatureT*>& features);
	
	void add_extra_features(const ProblemInfo& info, std::vector<FeatureT*>& features, FeatureIndex* featidx);
	
protected:
	const ProblemInfo& _info;
};


} } // namespaces
