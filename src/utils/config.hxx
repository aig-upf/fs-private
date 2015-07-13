
#pragma once

#include <stdexcept>

namespace fs0 {

/**
 */
class Config {
public:
	//! The type of relaxed plan extraction
	enum class RPGExtractionType {Propositional, Supported};
	
	//! The type of action manager
	enum class ActionManagerType {Gecode, Hybrid};
	
	//! The type of goal manager
	enum class GoalManagerType {Gecode, Hybrid, Basic};
	
	//! The possible types of CSP resolutions we consider
	enum class CSPResolutionType {Full, Approximate};
	
	//! Explicit initizalition of the singleton
	static void init(const std::string& filename);
	
	//! Retrieve the singleton instance, which has been previously initialized
	static Config& instance();
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const Config& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	
	static void validateConfig(const Config& config);
	
protected:
	static Config* _instance;
	
	std::string _filename;
	
	RPGExtractionType _rpg_extraction;
	
	ActionManagerType _action_manager;
	
	GoalManagerType _goal_manager;
	
	CSPResolutionType _goal_resolution;
	
	CSPResolutionType _precondition_resolution;
	
	Config(const std::string& filename);
	
	
public:
	~Config();
	
	const RPGExtractionType& getRPGExtractionType() const { return _rpg_extraction; }
	const ActionManagerType& getActionManagerType() const { return _action_manager; }
	const GoalManagerType& getGoalManagerType() const { return _goal_manager; }
	const CSPResolutionType& getGoalResolutionType() const { return _goal_resolution; }
	const CSPResolutionType& getActionPreconditionResolutionType() const { return _precondition_resolution; }
};

} // namespaces


