
#pragma once

#include <stdexcept>
#include <memory>
#include <unordered_map>
#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>

namespace fs0 {

//! A (singleton) object to load and store different planner configuration objects
class Config {
public:
	//! The type of relaxed plan extraction
	enum class RPGExtractionType {Propositional, Supported};
	
	//! The possible types of CSP resolutions we consider
	enum class CSPResolutionType {Full, Approximate};
	
	//! The CSP value selection heuristic
	enum class ValueSelection {MinVal, MinHMax};
	
	//! The type of support sets that should be given priority
	enum class SupportPriority {First, MinHMaxSum};
	
	//! The type of node evaluation
	enum class EvaluationT {eager, delayed, delayed_for_unhelpful};
	
	//! Explicit initizalition of the singleton
	static void init(const std::string& root, const std::unordered_map<std::string, std::string>& user_options, const std::string& filename);
	
	//! Retrieve the singleton instance, which has been previously initialized
	static Config& instance();

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const Config& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;

protected:
	static std::unique_ptr<Config> _instance;
	
	boost::property_tree::ptree _root;
	
	const std::unordered_map<std::string, std::string> _user_options;
	
	RPGExtractionType _rpg_extraction;
	
	CSPResolutionType _goal_resolution;
	
	CSPResolutionType _precondition_resolution;
	
	ValueSelection _goal_value_selection;
	
	ValueSelection _action_value_selection;
	
	SupportPriority _support_priority;
	
	bool _novelty;
	
	EvaluationT _node_evaluation;
	
	std::string _heuristic;
	
	//! Private constructor
	Config(const std::string& root, const std::unordered_map<std::string, std::string>& user_options, const std::string& filename);
	
public:
	Config(const Config& other) = delete;
	~Config() = default;
	
	void load(const std::string& filename);

	RPGExtractionType getRPGExtractionType() const { return _rpg_extraction; }
	
	CSPResolutionType getGoalResolutionType() const { return _goal_resolution; }
	
	CSPResolutionType getActionPreconditionResolutionType() const { return _precondition_resolution; }
	
	ValueSelection getGoalValueSelection() const { return _goal_value_selection; }
	
	ValueSelection getActionValueSelection() const { return _action_value_selection; }
	
	EvaluationT getNodeEvaluationType() const { return _node_evaluation; }
	
	bool useMinHMaxGoalValueSelector() const { return _goal_value_selection == ValueSelection::MinHMax; }
	
	bool useMinHMaxActionValueSelector() const { return _action_value_selection == ValueSelection::MinHMax; }
	
	bool useMinHMaxSumSupportPriority() const { return _support_priority == SupportPriority::MinHMaxSum; }
	
	bool useNoveltyConstraint() const { return _novelty; }
	
	const std::string& getHeuristic() const { return _heuristic; }
	
	bool useDelayedEvaluation() const { return _node_evaluation == EvaluationT::delayed; }
	
	bool useApproximateActionResolution() const {
		return getActionPreconditionResolutionType() == CSPResolutionType::Approximate;
	}
	
	bool useApproximateGoalResolution() const {
		return getGoalResolutionType() == CSPResolutionType::Approximate;
	}
	
	bool validate() const { return getOption("validate"); }
	
	//! A generic getter
	template <typename T>
	T getOption(const std::string& key) const {
		auto it = _user_options.find(key);
		if (it != _user_options.end()) { // The user specified an option value, which thus has priority
			return boost::lexical_cast<T>(it->second);
		} else {
			return _root.get<T>(key);
		}
	}

// 	template <>
	bool getOption(const std::string& key) const {
		auto it = _user_options.find(key);
		if (it != _user_options.end()) { // The user specified an option value, which thus has priority
			std::istringstream ss(it->second);
			bool b;
			ss >> std::boolalpha >> b;
			return b;
		} else {
			return _root.get<bool>(key);
		}
	}
};

} // namespaces


