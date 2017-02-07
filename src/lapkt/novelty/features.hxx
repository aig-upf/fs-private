
#pragma once

#include <vector>
#include <iostream>
#include <memory>


namespace lapkt { namespace novelty {

//! Features are (ATM) integer values
using FeatureValueT = int;

//! A feature valuation is an ordered set of feature values
using FeatureValuation = std::vector<FeatureValueT>;
using IntFeatureValuation = std::vector<int>;
using BinaryFeatureValuation = std::vector<bool>;

//! Base interface. A single novelty feature basically takes a state of a given type and returns a feature value.
template <typename StateT>
class NoveltyFeature {
public:
	virtual ~NoveltyFeature() = default;
	virtual NoveltyFeature* clone() const = 0;
	
	//!
	virtual FeatureValueT evaluate(const StateT& s) const = 0;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const NoveltyFeature& o) { return o.print(os); }
	virtual std::ostream& print(std::ostream& os) const = 0;
};


//! An (ordered) set of generic novelty features
template <typename StateT>
class FeatureSetEvaluator {
public:
	using FeatureT = std::unique_ptr<NoveltyFeature<StateT>>;
	
	FeatureSetEvaluator() = default;
	~FeatureSetEvaluator() = default;
	
	FeatureSetEvaluator(const FeatureSetEvaluator&) = delete;
	FeatureSetEvaluator(FeatureSetEvaluator&&) = default;
	FeatureSetEvaluator& operator=(const FeatureSetEvaluator&) = delete;
	FeatureSetEvaluator& operator=(FeatureSetEvaluator&&) = default;
	
	//!
	void add(FeatureT&& feature) {
		_features.push_back(std::move(feature));
	}
	
	//!
	FeatureValuation evaluate(const StateT& state) const {
	// 	LPT_INFO("novelty-evaluations", "Evaluating state " << state);
		FeatureValuation values;
		
		values.reserve(_features.size());
		for (const auto& feature:_features) {
			values.push_back(feature->evaluate(state));
			// LPT_INFO("novelty-evaluations", "\t" << _featureMap.feature(k) << ": " << values[k]);
		}

	// 	LPT_DEBUG("heuristic", "Feature evaluation: " << std::endl << print::feature_set(varnames, values));
		return values;
	}
	
	//! The number of features in the set
	unsigned size() const { return _features.size(); }
	
protected:
	//! The features in the set
	std::vector<FeatureT> _features;
};

//! A "straight" evaluator simply returns as features the binary value of each of the binary state variables of the state.
//! This should be more performant, as it simply returns a const reference to the 
//! (already existing in the state) vector of boolean values.
template <typename StateT>
class StraightBinaryFeatureSetEvaluator {
public:
	//!
	const BinaryFeatureValuation& evaluate(const StateT& state) const { return state.get_boolean_values(); }
};

template <typename StateT>
class StraightMultivaluedFeatureSetEvaluator {
public:
	//!
	const IntFeatureValuation& evaluate(const StateT& state) const { return state.get_int_values(); }
};


//! A feature evaluator that works for hybrid states
template <typename StateT>
class StraightHybridFeatureSetEvaluator {
public:
	//!
	const IntFeatureValuation evaluate(const StateT& state) const {
		unsigned sz = state.numAtoms();
		IntFeatureValuation valuation;
		valuation.reserve(sz);
		
		for (unsigned var = 0; var < sz; ++var) {
			valuation.push_back(state.getValue(var));
		}
		return valuation;
	}
};

} } // namespaces
