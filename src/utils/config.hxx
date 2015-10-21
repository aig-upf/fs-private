
#pragma once

#include <stdexcept>
#include <memory>

#include <boost/property_tree/ptree.hpp>


namespace fs0 {

/**
 */
class Config {
public:
	//! The type of relaxed plan extraction
	enum class RPGExtractionType {Propositional, Supported};
	
	//! The type of CSP manager
	enum class CSPManagerType {Gecode, Direct, DirectIfPossible};
	
	//! The type of CSP model (gecode manager only)
	enum class CSPModel {ActionCSP, EffectCSP};
	
	//! The possible types of CSP resolutions we consider
	enum class CSPResolutionType {Full, Approximate};
	
	//! The CSP value selection heuristic
	enum class ValueSelection {MinVal, MinHMax};
	
	//! The type of support sets that should be given priority
	enum class SupportPriority {First, MinHMaxSum};
	
	//! Explicit initizalition of the singleton
	static void init(const std::string& filename);
	
	//! Retrieve the singleton instance, which has been previously initialized
	static Config& instance();

	//! Some rudimentary validation of the compatibility of different configuration options
	static void validateConfig(const Config& config);
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const Config& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;

protected:
	static std::unique_ptr<Config> _instance;
	
	boost::property_tree::ptree _root;
	
	std::string _filename;
	
	//! The tag that identifies the search engine we want to use
	std::string _engine_tag;
	
	RPGExtractionType _rpg_extraction;
	
	CSPManagerType _csp_manager;
	
	CSPModel _csp_model;
	
	CSPResolutionType _goal_resolution;
	
	CSPResolutionType _precondition_resolution;
	
	ValueSelection _goal_value_selection;
	ValueSelection _action_value_selection;
	
	SupportPriority _support_priority;
	
	bool _novelty_constraint;
	
	bool _element_dont_care_optimization;
	
	//! Private constructor
	Config(const std::string& filename);
	
public:
	Config(const Config& other) = delete;
	~Config();
	
	const std::string& getEngineTag() const { return _engine_tag; }
	
	RPGExtractionType getRPGExtractionType() const { return _rpg_extraction; }
	
	CSPManagerType getCSPManagerType() const { return _csp_manager; }
	
	CSPModel getCSPModel() const { return _csp_model; }
	
	CSPResolutionType getGoalResolutionType() const { return _goal_resolution; }
	
	CSPResolutionType getActionPreconditionResolutionType() const { return _precondition_resolution; }
	
	ValueSelection getGoalValueSelection() const { return _goal_value_selection; }
	
	ValueSelection getActionValueSelection() const { return _action_value_selection; }
	
	bool useMinHMaxGoalValueSelector() const { return _goal_value_selection == ValueSelection::MinHMax; }
	
	bool useMinHMaxActionValueSelector() const { return _action_value_selection == ValueSelection::MinHMax; }
	
	bool useMinHMaxSumSupportPriority() const { return _support_priority == SupportPriority::MinHMaxSum; }
	
	bool useNoveltyConstraint() const { return _novelty_constraint; }
	
	bool useElementDontCareOptimization() const { return _element_dont_care_optimization; }
	
	bool useApproximateActionResolution() const {
		return getActionPreconditionResolutionType() == CSPResolutionType::Approximate;
	}

	//! A generic getter
	template <typename T>
	T getOption(const std::string& key) const {
		return _root.get<T>(key);
	}
};

} // namespaces


