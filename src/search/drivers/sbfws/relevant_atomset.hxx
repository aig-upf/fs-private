
#pragma once

#include <lapkt/tools/logging.hxx>

#include <utils/atom_index.hxx>
#include <utils/utils.hxx>

#include <state.hxx>
#include <problem_info.hxx>

#include <boost/functional/hash.hpp>
#include <unordered_set>


namespace fs0 { namespace bfws {

 //! A helper object to reduce the memory footprint of RelevantAtomSets
class AtomsetHelper {
public:
	typedef std::pair< FeatureIdx, int >    ValuationT;
	typedef std::tuple< FeatureIdx, int, FeatureIdx, int >    CoupledValuationT;


	//! '_relevant[i]' iff the atom with index 'i' is relevant
	const std::vector<bool> _relevant;
	const std::unordered_set<ValuationT,boost::hash<ValuationT>> _relevant_feature_values;
	const std::unordered_set<CoupledValuationT,boost::hash<CoupledValuationT>> _relevant_coupled_feature_values;


	//! The number of relevant atoms, i.e. of 'true' values in _relevant
	const unsigned _num_relevant;

	//! A reference to the global atom index
	const AtomIndex& _atomidx;

	AtomsetHelper(const AtomIndex& atomidx, const std::vector<bool>& relevant) :
		_relevant(relevant), _num_relevant(std::count(relevant.begin(), relevant.end(), true)), _atomidx(atomidx)
	{}

	AtomsetHelper(const AtomIndex& atomidx, const std::vector<bool>& relevant, const std::vector<bool>& dummy ) :
		_relevant(relevant), _num_relevant(std::count(relevant.begin(), relevant.end(), true)), _atomidx(atomidx)
	{}


	AtomsetHelper(const AtomIndex& atomidx, const std::vector<ValuationT>& relevant) :
		_relevant_feature_values(relevant.begin(), relevant.end()), _num_relevant(_relevant_feature_values.size()), _atomidx(atomidx)
	{}

	AtomsetHelper(const AtomIndex& atomidx, const std::vector<ValuationT>& relevant, const std::vector<CoupledValuationT>& coupled ) :
		_relevant_feature_values(relevant.begin(), relevant.end()),
		_relevant_coupled_feature_values(coupled.begin(), coupled.end()),
		_num_relevant(_relevant_feature_values.size() + coupled.size()), _atomidx(atomidx)
	{}

	unsigned size() const { return _atomidx.size(); }
};

//! A RelevantAtomSet contains information about which of the atoms of a problem are relevant for a certain
//! goal, and, among those, which have already been reached and which others have not.
class RelevantAtomSet {
public:
    typedef std::pair< FeatureIdx, int >    ValuationT;
    typedef std::unordered_set< ValuationT, boost::hash<ValuationT> >        ValuationSet;
	typedef std::tuple< FeatureIdx, int, FeatureIdx, int >    CoupledValuationT;
    typedef std::unordered_set< CoupledValuationT, boost::hash<CoupledValuationT> >        CoupledValuationSet;


	//! A RelevantAtomSet is always constructed with all atoms being marked as IRRELEVANT
	RelevantAtomSet(const AtomsetHelper& helper) :
		_helper(helper), _num_reached(0), _bool_reached(helper.size(), false), _int_reached() //, _updated(false)
	{}

	~RelevantAtomSet() = default;
	RelevantAtomSet(const RelevantAtomSet&) = default;
	RelevantAtomSet(RelevantAtomSet&&) = default;
	RelevantAtomSet& operator=(const RelevantAtomSet&) = default;
	RelevantAtomSet& operator=(RelevantAtomSet&&) = default;


	//! Update those atoms that have been reached in the given state
	void update(const State& state, const State* parent) {
// 		assert(!_updated); // Don't want to update a node twice!
// 		_updated = true;
		unsigned n = state.numAtoms();
		for (VariableIdx var = 0; var < n; ++var) {
			object_id val = state.getValue(var);
			if (parent && (val == parent->getValue(var))) continue; // If a parent was provided, we check that the value is new wrt the parent

			if ( ProblemInfo::getInstance().sv_type(var) == type_id::float_t
			|| ProblemInfo::getInstance().sv_type(var) == type_id::int_t ) {
				auto hint = _int_reached.insert(ValuationT(var,val.value()));
				if (hint.second) { ++_num_reached; }
				continue;
			}

			if (!_helper._atomidx.is_indexed(var, val)) continue;

			AtomIdx atom = _helper._atomidx.to_index(var, val);
			if (!_helper._relevant[atom]) continue; // we're not concerned about this atom

			std::vector<bool>::reference ref = _bool_reached[atom];
			if (!ref) {
				++_num_reached;
				ref = true;
			}
		}
	}

	unsigned num_reached() const { return _num_reached; }

	template <typename FeatureValuationT>
	void init( const FeatureValuationT& phi ) {
		_int_reached = ValuationSet();
		_num_reached = 0;
		update(phi);
		_num_reached = 0;
	}

	template <typename FeatureValuationT>
	void update(const FeatureValuationT& phi) {
		for (unsigned k = 0; k < phi.size(); k++ ) {
			ValuationT v(k, phi[k]);
			if ( _helper._relevant_feature_values.find(v) == _helper._relevant_feature_values.end() )
				continue;
			auto hint = _int_reached.insert(v);
			if (hint.second) { ++_num_reached; }
		}

		for ( auto t : _helper._relevant_coupled_feature_values ) {
			FeatureIdx k0, k1;
			int v0, v1;
			std::tie( k0, v0, k1, v1) = t;
			if ( phi[k0] != v0 || phi[k1] != v1) continue;
			auto hint = _pair_reached.insert( t );
			if ( hint.second ) { ++_num_reached; }
		}
	}

	template <typename FeatureValuationT>
	void init_from_subset( const FeatureValuationT& phi ) {
		_int_reached = ValuationSet();
		_num_reached = 0;
		update_from_subset(phi);
		_num_reached = 0;
	}

	void update_from_subset(const std::vector<ValuationT>& phi) {
		for (unsigned k = 0; k < phi.size(); k++ ) {
			if ( _helper._relevant_feature_values.find(phi[k]) == _helper._relevant_feature_values.end() )
				continue;
			auto hint = _int_reached.insert(phi[k]);
			if (hint.second) { ++_num_reached; }
		}
		for ( auto t : _helper._relevant_coupled_feature_values ) {
			FeatureIdx k0, k1;
			int v0, v1;
			std::tie( k0, v0, k1, v1) = t;
			if ( std::find( phi.begin(), phi.end(), std::make_pair(k0,v0)) == phi.end()
		 		|| std::find( phi.begin(), phi.end(), std::make_pair(k1,v1)) == phi.end())
				continue;
			auto hint = _pair_reached.insert( t );
			if ( hint.second ) { ++_num_reached; }
		}
	}

	void update_from_subset( const std::vector<bool>& phi );

	void update_from_subset( const std::vector<int>& phi );


	void init(const State& state) {
		_bool_reached = std::vector<bool>(_helper.size(), false);
        _int_reached = ValuationSet();
		_num_reached = 0;
		update(state, nullptr);
 		assert(_num_reached == std::count(_bool_reached.begin(), _bool_reached.end(), true));
 		_num_reached = 0;
	}

	//! Prints a representation of the state to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const RelevantAtomSet& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const {
		if (_helper._relevant.size() == 0 )
			return os << "{ }";

		const AtomIndex& atomidx = _helper._atomidx;

		os << "{";
		for (unsigned i = 0; i < _bool_reached.size(); ++i) {
			const Atom& atom = atomidx.to_atom(i);
			if (!_helper._relevant[i]) continue;

			std::string mark = (_bool_reached[i]) ? "*" : "";
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

	//! _bool_reached[i] iff atom with index 'i' has been reached at some point
	//! since the count of reached subgoals was last increased.
	std::vector<bool> _bool_reached;

    ValuationSet 			_int_reached;
	CoupledValuationSet		_pair_reached;

// 	bool _updated;
};



} } // namespaces
