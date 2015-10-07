
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
	static std::unique_ptr<Config> _instance;
	
	boost::property_tree::ptree _root;
	
	std::string _filename;
	
	//! The tag that identifies the search engine we want to use
	std::string _engine_tag;
	
	RPGExtractionType _rpg_extraction;
	
	CSPManagerType _csp_manager;
	
	CSPResolutionType _goal_resolution;
	
	CSPResolutionType _precondition_resolution;
	
	Config(const std::string& filename);
	
	
public:
	Config(const Config& other) = delete;
	~Config();
	
	const std::string& getEngineTag() const { return _engine_tag; }
	const RPGExtractionType& getRPGExtractionType() const { return _rpg_extraction; }
	const CSPManagerType& getCSPManagerType() const { return _csp_manager; }
	const CSPResolutionType& getGoalResolutionType() const { return _goal_resolution; }
	const CSPResolutionType& getActionPreconditionResolutionType() const { return _precondition_resolution; }

	//! A generic getter
	template <typename T>
	T getOption(const std::string& key) const {
		return _root.get<T>(key);
	}
};

} // namespaces


