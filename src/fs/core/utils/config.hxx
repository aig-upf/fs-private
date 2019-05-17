
#pragma once

#include <stdexcept>
#include <memory>
#include <unordered_map>
#include <boost/property_tree/ptree.hpp>
#include <fs/core/utils/lexical_cast.hxx>


namespace fs0 {

class MissingOption : public std::runtime_error {
public:
	MissingOption(const std::string& name) : std::runtime_error("Missing or Wrong option type: '" + name + "'") {}
};


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

	//! The type of successor generator to use
	enum class SuccessorGenerationStrategy { naive, functional_aware, match_tree, adaptive };

    //! Integrator
	enum class IntegratorT { ExplicitEuler, RungeKutta2, RungeKutta4, ImplicitEuler };

	//! Explicit initizalition of the singleton
	static void init(const std::string& root, const std::unordered_map<std::string, std::string>& user_options, const std::string& filename);

	//! Retrieve the singleton instance, which has been previously initialized
	static Config& instance();
	static void setAsGlobal( std::unique_ptr<Config>&& ptr );

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

	SuccessorGenerationStrategy	_succ_gen_type;

    IntegratorT	_successor_prediction;

    double _integration_factor;

    double _discretization_step;

    bool _zero_crossing_control;

    double _horizon_time;

	//! Private constructor

public:
	Config(const std::string& root, const std::unordered_map<std::string, std::string>& user_options, const std::string& filename);

	Config(const Config& other) = delete;
	~Config() = default;

	void load(const std::string& filename);

	RPGExtractionType getRPGExtractionType() const { return _rpg_extraction; }

	CSPResolutionType getGoalResolutionType() const { return _goal_resolution; }

	CSPResolutionType getActionPreconditionResolutionType() const { return _precondition_resolution; }

	ValueSelection getGoalValueSelection() const { return _goal_value_selection; }

	ValueSelection getActionValueSelection() const { return _action_value_selection; }

	EvaluationT getNodeEvaluationType() const { return _node_evaluation; }

	SuccessorGenerationStrategy getSuccessorGeneratorType() const { return _succ_gen_type; }

	bool requiresHelpfulnessAssessment() const {
		return _node_evaluation ==  EvaluationT::delayed_for_unhelpful
				|| (getOption<bool>("helpful_actions") && getOption<bool>("ehc"));
	}

	bool useMinHMaxGoalValueSelector() const { return _goal_value_selection == ValueSelection::MinHMax; }

	bool useMinHMaxActionValueSelector() const { return _action_value_selection == ValueSelection::MinHMax; }

	bool useMinHMaxSumSupportPriority() const { return _support_priority == SupportPriority::MinHMaxSum; }

	bool useNoveltyConstraint() const { return _novelty; }

	const std::string& getHeuristic() const { return _heuristic; }

	bool useApproximateActionResolution() const {
		return getActionPreconditionResolutionType() == CSPResolutionType::Approximate;
	}

	bool useApproximateGoalResolution() const {
		return getGoalResolutionType() == CSPResolutionType::Approximate;
	}

    void setSuccessorPredictionType( IntegratorT method ) { _successor_prediction = method; }

    IntegratorT getSuccessorPredictionType() const { return _successor_prediction; }

    void setIntegrationFactor( double nsteps ) { _integration_factor = nsteps; }

    double getIntegrationFactor( ) const { return _integration_factor; }

    void setDiscretizationStep( double dT ) { _discretization_step = dT; }

    double getDiscretizationStep( ) const { return _discretization_step; }

    bool getZeroCrossingControl() const { return _zero_crossing_control; }
	void setZeroCrossingControl( bool do_zcc ) { _zero_crossing_control = do_zcc; }

	void setHorizonTime( double H ) { _horizon_time = H;	}
    double getHorizonTime() const { return _horizon_time; }
    bool hasHorizon() const { return _horizon_time >= 1e-7; }

	bool validate() const { return getOption("validate", false); }

	//! A generic getter
	template <typename T>
	T getOption(const std::string& key) const {
		auto it = _user_options.find(key);
		try {
			if (it != _user_options.end()) { // The user specified an option value, which thus has priority
				return boost::lexical_cast<T>(it->second);
			} else {
				return _root.get<T>(key);
			}
		} catch (const std::runtime_error& e) {
			throw MissingOption(key);
		}
	}

	template <typename T>
	T getOption(const std::string& key, const T& def) const {
		try {
			return getOption<T>(key);
		} catch (const MissingOption& e) {
			return def;
		}
	}

	// Partial specialization
	bool getOption(const std::string& key) const { return getOption<bool>(key); }
	bool getOption(const std::string& key, bool def) const { return getOption<bool>(key, def); }
};

} // namespaces
