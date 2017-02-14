
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

} } // namespaces
