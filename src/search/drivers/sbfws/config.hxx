
#pragma once


namespace fs0 { class Config; }

namespace fs0 { namespace bfws {

//! A configuration class for the SBFWS search
struct SBFWSConfig {
	SBFWSConfig(const Config&);
	SBFWSConfig(const SBFWSConfig&) = default;
	SBFWSConfig& operator=(const SBFWSConfig&) = default;
	SBFWSConfig(SBFWSConfig&&) = default;
	SBFWSConfig& operator=(SBFWSConfig&&) = default;

	//! The maximum levels of width for search and simulation
	const unsigned search_width;
	const unsigned simulation_width;
	const bool mark_negative_propositions;
	const bool complete_simulation;
	const bool using_feature_set;
	
	enum class NoveltyEvaluatorType {Adaptive, Generic};
	NoveltyEvaluatorType evaluator_t;

	enum class RelevantSetType {None, Sim, L0, G0};
	RelevantSetType relevant_set_type;

	//! When to recompute R: only in seed node, or each time number of unachieved goals #g decreases
	enum class RComputation {Seed, GDecr};
	RComputation r_computation;
	
	const Config& _global_config;
};

} } // namespaces
