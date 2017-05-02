
#pragma once

#include <lapkt/tools/logging.hxx>

#include <utils/atom_index.hxx>
#include <state.hxx>


namespace fs0 { namespace bfws {

 //! A helper object to reduce the memory footprint of RelevantAtomSets
class AtomsetHelper {
public:
	//! '_relevant[i]' iff the atom with index 'i' is relevant
	const std::vector<bool> _relevant;

	//! The number of relevant atoms, i.e. of 'true' values in _relevant
	const unsigned _num_relevant;

	//! A reference to the global atom index
	const AtomIndex& _atomidx;

	AtomsetHelper(const AtomIndex& atomidx, const std::vector<bool>& relevant) :
		_relevant(relevant), _num_relevant(std::count(relevant.begin(), relevant.end(), true)), _atomidx(atomidx)
	{}

	unsigned size() const { return _atomidx.size(); }
};

//! A RelevantAtomSet contains information about which of the atoms of a problem are relevant for a certain
//! goal, and, among those, which have already been reached and which others have not.
class RelevantAtomSet {
public:

	//! A RelevantAtomSet is always constructed with all atoms being marked as IRRELEVANT
	RelevantAtomSet(const AtomsetHelper& helper) :
		_helper(helper), _num_reached(0), _reached(helper.size(), false) //, _updated(false)
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
			ObjectIdx val = boost::get<int>(state.getValue(var));
			if (parent && (val == parent->getValue(var))) continue; // If a parent was provided, we check that the value is new wrt the parent

			AtomIdx atom = _helper._atomidx.to_index(var, val);
			if (!_helper._relevant[atom]) continue; // we're not concerned about this atom

			std::vector<bool>::reference ref = _reached[atom];
			if (!ref) {
				++_num_reached;
				ref = true;
			}
		}
	}

	unsigned num_reached() const { return _num_reached; }

	void init(const State& state) {
		_reached = std::vector<bool>(_helper.size(), false);
		_num_reached = 0;
		update(state, nullptr);
 		assert(_num_reached == std::count(_reached.begin(), _reached.end(), true));
 		_num_reached = 0;
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

	//! _reached[i] iff atom with index 'i' has been reached at some point
	//! since the count of reached subgoals was last increased.
	std::vector<bool> _reached;

// 	bool _updated;
};

} } // namespaces
