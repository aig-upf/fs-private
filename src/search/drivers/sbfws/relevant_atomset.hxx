
#pragma once

#include <iomanip>

#include <aptk2/tools/logging.hxx>
#include <problem_info.hxx>
#include <utils/printers/relevant_atomset.hxx>
#include <utils/atom_index.hxx>
#include <state.hxx>


namespace fs0 { namespace bfws {

//! A RelevantAtomSet contains information about which of the atoms of a problem are relevant for a certain
//! goal, and, among those, which have already been reached and which others have not.
class RelevantAtomSet {
public:
	enum class STATUS : unsigned char {IRRELEVANT, UNREACHED, REACHED};

	//! A RelevantAtomSet is always constructed with all atoms being marked as IRRELEVANT
	RelevantAtomSet(const AtomIndex* atomidx) :
		_atomidx(atomidx), _num_reached(0), _num_unreached(0), _status(atomidx ? atomidx->size() : 0, STATUS::IRRELEVANT)
	{}

	~RelevantAtomSet() = default;
	RelevantAtomSet(const RelevantAtomSet&) = default;
	RelevantAtomSet(RelevantAtomSet&&) = default;
	RelevantAtomSet& operator=(const RelevantAtomSet&) = default;
	RelevantAtomSet& operator=(RelevantAtomSet&&) = default;

	//! Marks all the atoms in the state with the given 'status'.
	//! If 'mark_negative_propositions' is false, predicative atoms of the form X=false are ignored
	//! If 'only_if_relevant' is true, only those atoms that were not deemed _irrelevant_ (i.e. their status was either reached or unreached)
	//! are marked
	void mark(const State& state, const State* parent, STATUS status, bool mark_negative_propositions, bool only_if_relevant) {
		assert(_atomidx);
		const ProblemInfo& info = ProblemInfo::getInstance();
		unsigned n = state.numAtoms();
		for (VariableIdx var = 0; var < n; ++var) {
			ObjectIdx val = state.getValue(var);
			if (!mark_negative_propositions && info.isPredicativeVariable(var) && val==0) continue; // We don't want to mark negative propositions
			if (parent && (val == parent->getValue(var))) continue; // If a parent was provided, we check that the value is new wrt the parent
			mark(_atomidx->to_index(var, val), status, only_if_relevant);
		}
	}

	//! Marks the atom with given index with the given status.
	//! If 'only_if_relevant' is true, then only marks the atom if it was previously
	//! marked as relevant (i.e. its status was _not_ STATUS::IRRELEVANT).
	void mark(AtomIdx idx, STATUS status, bool only_if_relevant) {
		assert(status==STATUS::REACHED || status==STATUS::UNREACHED);
		auto& st = _status[idx];
		if (only_if_relevant && (st == STATUS::IRRELEVANT)) return;

		if (st != status) {
			if (status==STATUS::REACHED) ++_num_reached;
			else if (status==STATUS::UNREACHED) ++_num_unreached;
			
			if (st==STATUS::REACHED) --_num_reached; // The old status was reached, and will not be anymore, so we decrease the counter.
			else if (st==STATUS::UNREACHED) --_num_unreached;
			
			st = status;
		}
	}

	unsigned num_reached() const { return _num_reached; }
	unsigned num_unreached() const { return _num_unreached; }
	
	bool valid() const { return _atomidx != nullptr; }

	friend class print::relevant_atomset;

protected:
	//! A reference to the global atom index
	const AtomIndex* _atomidx;

	//! The total number of reached / unreached atoms
	unsigned _num_reached;
	unsigned _num_unreached;

	//! The status of each atom (indexed by its atom index)
	std::vector<STATUS> _status;
};

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

//! A LightRelevantAtomSet contains information about which of the atoms of a problem are relevant for a certain
//! goal, and, among those, which have already been reached and which others have not.
class LightRelevantAtomSet {
public:

	//! A LightRelevantAtomSet is always constructed with all atoms being marked as IRRELEVANT
	LightRelevantAtomSet(const AtomsetHelper& helper) :
		_helper(helper), _num_reached(0), _reached(helper.size(), false) //, _updated(false)
	{}

	~LightRelevantAtomSet() = default;
	LightRelevantAtomSet(const LightRelevantAtomSet&) = default;
	LightRelevantAtomSet(LightRelevantAtomSet&&) = default;
	LightRelevantAtomSet& operator=(const LightRelevantAtomSet&) = default;
	LightRelevantAtomSet& operator=(LightRelevantAtomSet&&) = default;
	
	
	//! Update those atoms that have been reached in the given state
	void update(const State& state, const State* parent) {
// 		assert(!_updated); // Don't want to update a node twice!
// 		_updated = true;
		unsigned n = state.numAtoms();
		for (VariableIdx var = 0; var < n; ++var) {
			ObjectIdx val = state.getValue(var);
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
// 	unsigned num_unreached() const { return _helper._num_relevant - _num_reached; }

	void init(const State& state) {
		_reached = std::vector<bool>(_helper.size(), false);
		_num_reached = 0;
		update(state, nullptr);
 		assert(_num_reached = std::count(_reached.begin(), _reached.end(), true));
 		_num_reached = 0;
	}
	
// 	bool valid() const { return _updated; }
	

// 	friend class print::relevant_atomset;
	//! Prints a representation of the state to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const LightRelevantAtomSet& o) { return o.print(os); }
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
