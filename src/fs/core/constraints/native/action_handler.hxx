
#pragma once

#include <fs/core/fs_types.hxx>
#include <fs/core/languages/fstrips/language_fwd.hxx>
#include <fs/core/languages/fstrips/effects.hxx>

namespace fs0 {
class GroundAction;
class AtomIndex;
}

namespace fs0 { namespace gecode {

class RPGIndex;


class SimpleFormulaChecker {
public:
    using SupportT = std::vector<AtomIdx>;

    SimpleFormulaChecker(const fs::Formula* formula, const AtomIndex& tuple_index);

    bool check_reachable(const RPGIndex& graph, SupportT& support) const;

protected:
    const AtomIndex& _tuple_index;

    //! The equality and inequality atoms in the precondition of the action
    std::vector<std::pair<VariableIdx, object_id>> _equality_atoms;
    std::vector<std::pair<VariableIdx, object_id>> _inequality_atoms;

};

//!
class NativeActionHandler {
public:
	//! Factory method
	static std::vector<std::unique_ptr<NativeActionHandler>> create(const std::vector<const GroundAction*>& actions, const AtomIndex& tuple_index);

	//! Constructors / Destructor
	NativeActionHandler(const GroundAction& action, const AtomIndex& tuple_index);
	~NativeActionHandler() = default;
	NativeActionHandler(const NativeActionHandler&) = delete;
	NativeActionHandler(NativeActionHandler&&) = delete;
	NativeActionHandler& operator=(const NativeActionHandler&) = delete;
	NativeActionHandler& operator=(NativeActionHandler&&) = delete;
	
	const std::vector<const fs::ActionEffect*>& get_effects() const;

	const fs::Formula* get_precondition() const;

	void process(RPGIndex& graph);

protected:

    const AtomIndex& _tuple_index;

	const GroundAction& _action;

    SimpleFormulaChecker _precondition_checker;
	
	std::vector<const fs::ActionEffect*> _add_effects;

	//! The equality and inequality atoms in the precondition of the action
    std::vector<std::pair<VariableIdx, object_id>> _equality_atoms;
    std::vector<std::pair<VariableIdx, object_id>> _inequality_atoms;

    std::vector<AtomIdx> _directly_achievable_tuples;
    std::vector<std::pair<VariableIdx, VariableIdx>> _lhs_rhs_statevars;


    //! Log some handler-related into
	virtual void log() const;
};


} } // namespaces
