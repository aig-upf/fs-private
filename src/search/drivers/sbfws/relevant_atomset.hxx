
#pragma once

#include <lapkt/tools/logging.hxx>

#include <utils/atom_index.hxx>
#include <state.hxx>

// using RelevantFeatureSet = std::unordered_set<std::pair<unsigned, int>,  boost::hash<std::pair<unsigned, int>> >;
using RelevantFeatureSet = std::vector<bool>;


namespace fs0 { namespace bfws {
	
class FeatureIndex {
public:	
	
	FeatureIndex() {}
	
	unsigned to_feature_index(unsigned feature_index, int feature_value) const {
		auto it = _feature_index.find(std::make_pair(feature_index, feature_value));
		if (it == _feature_index.end()) throw std::runtime_error("Unknown feature value pair");
		return it->second;
	}
	
	unsigned num_feature_indexes() const { return _feature_index.size(); }
	
	unsigned add(unsigned idx, int val) {
		auto pair = std::make_pair(idx, val);
		auto it = _feature_index.find(pair);
		if (it != _feature_index.end()) throw std::runtime_error("Duplicated feature index");
		
		unsigned n = _feature_index.size();
		_feature_index.insert(std::make_pair(pair, n));
		return  n;
	}
	
protected:
	using FeatureValuePair = std::pair<unsigned, int>;
	std::unordered_map<FeatureValuePair, unsigned, boost::hash<FeatureValuePair>> _feature_index;
};	

 //! A helper object to reduce the memory footprint of RelevantAtomSets
class AtomsetHelper {
public:
	//! '_relevant[i]' iff the atom with index 'i' is relevant
	const std::vector<bool> _relevant;

	//! The number of relevant atoms, i.e. of 'true' values in _relevant
	const unsigned _num_relevant;

	//! A reference to the global atom index
	const AtomIndex& _atomidx;
	
	const FeatureIndex& _featidx;
	
	RelevantFeatureSet _relevant_features;

	AtomsetHelper(const AtomIndex& atomidx, const std::vector<bool>& relevant, const FeatureIndex& featidx, const RelevantFeatureSet& F) :
		_relevant(relevant), _num_relevant(std::count(relevant.begin(), relevant.end(), true)), _atomidx(atomidx), _featidx(featidx), _relevant_features(F)
	{}

	unsigned size() const { return _atomidx.size(); }
};

//! A RelevantAtomSet contains information about which of the atoms of a problem are relevant for a certain
//! goal, and, among those, which have already been reached and which others have not.
class RelevantAtomSet {
public:

	//! A RelevantAtomSet is always constructed with all atoms being marked as IRRELEVANT
	RelevantAtomSet(const AtomsetHelper& helper) :
		_helper(helper), _num_reached(0), _num_reachedF(0), _reached(helper.size(), false), _reachedF(helper._featidx.num_feature_indexes(), false) //, _updated(false)
	{}

	~RelevantAtomSet() = default;
	RelevantAtomSet(const RelevantAtomSet&) = default;
	RelevantAtomSet(RelevantAtomSet&&) = default;
	RelevantAtomSet& operator=(const RelevantAtomSet&) = default;
	RelevantAtomSet& operator=(RelevantAtomSet&&) = default;


	//! Update those atoms that have been reached in the given state
	template <typename FeatureValueT>
	void update(const State& state, const State* parent, const std::vector<FeatureValueT>& feature_valuation) {
// 		assert(!_updated); // Don't want to update a node twice!
// 		_updated = true;
		unsigned n = state.numAtoms();
		for (VariableIdx var = 0; var < n; ++var) {
			ObjectIdx val = state.getValue(var);
			if (parent && (val == parent->getValue(var))) continue; // If a parent was provided, we check that the value is new wrt the parent

			if (!_helper._atomidx.is_indexed(var, val)) continue;
			
			AtomIdx atom = _helper._atomidx.to_index(var, val);
			if (!_helper._relevant[atom]) continue; // we're not concerned about this atom

			std::vector<bool>::reference ref = _reached[atom];
			if (!ref) {
				++_num_reached;
				ref = true;
			}
		}

		unsigned feat_idx = state.numAtoms(); // start the iteration ignoring the first n features, which are simple state variables
		for (; feat_idx < feature_valuation.size(); ++feat_idx) {
				
			unsigned feature_index = _helper._featidx.to_feature_index(feat_idx, feature_valuation[feat_idx]);
			if (!_helper._relevant_features[feature_index]) continue; // we're not concerned about this atom

			std::vector<bool>::reference ref = _reachedF[feature_index];
			if (!ref) {
				++_num_reachedF;
				ref = true;
			}			
		}
	}

	unsigned num_reached() const { return _num_reached; }
	unsigned num_reachedF() const { return _num_reachedF; }

	template <typename FeatureValueT>
	void init(const State& state, const std::vector<FeatureValueT>& feature_valuation) {
		_reached = std::vector<bool>(_helper.size(), false);
		_reachedF = std::vector<bool>(_helper._featidx.num_feature_indexes(), false);
		_num_reached = 0;
		_num_reachedF = 0;
		update(state, nullptr, feature_valuation);
 		assert(_num_reached == std::count(_reached.begin(), _reached.end(), true));
 		_num_reached = 0;
		_num_reachedF = 0;
	}

	//! Prints a representation of the state to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const RelevantAtomSet& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const {
		const AtomIndex& atomidx = _helper._atomidx;

		os << "{";
		for (unsigned i = 0; i < _reached.size(); ++i) {
			const Atom& atom = atomidx.to_atom(i);
			if (!_helper._relevant[i]) continue;

			std::string mark = (_reached[i]) ? "*" : "";
			os << atom << mark << ", ";
		}
		os << "}";

		return os;
	}

	const AtomsetHelper& getHelper() const { return _helper; }

protected:
	//! A reference to the global atom index
	const AtomsetHelper& _helper;

	//! The total number of reached / unreached atoms
	unsigned _num_reached;
	
	unsigned _num_reachedF;

	//! _reached[i] iff atom with index 'i' has been reached at some point
	//! since the count of reached subgoals was last increased.
	std::vector<bool> _reached;
	
	RelevantFeatureSet _reachedF;

// 	bool _updated;
};

} } // namespaces
