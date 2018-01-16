
#pragma once

#include <fs/core/constraints/gecode/handlers/formula_csp.hxx>
#include <fs/core/monotonicity.hxx>

namespace fs0 { class AtomIndex; }
namespace fs0 { namespace language { namespace fstrips { class Formula; }}}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {
	
//! A CSP modeling and solving a logical formula on a certain RPG layer
class MonotonicityCSP : public FormulaCSP {
public:

	MonotonicityCSP(const fs::Formula* formula, const AtomIndex& tuple_index, const AllTransitionGraphsT& transitions, bool approximate);
	~MonotonicityCSP() = default;
	MonotonicityCSP(const MonotonicityCSP&) = delete;
	MonotonicityCSP(MonotonicityCSP&&) = delete;
	MonotonicityCSP& operator=(const MonotonicityCSP&) = delete;
	MonotonicityCSP& operator=(MonotonicityCSP&&) = delete;

	GecodeCSP* instantiate_from_changeset(const GecodeCSP& parent_csp, const State& state, const std::vector<Atom>& changeset) const;

    GecodeCSP* check(const GecodeCSP& parent_csp, const State* parent, const State& child, const std::vector<Atom>& changeset) const;

    GecodeCSP* check_consistency(const GecodeCSP& parent_csp, const State& state, const std::vector<Atom>& changeset) const;

    static GecodeCSP* solve_csp(GecodeCSP* csp);

protected:
	const TransitionGraph _monotonicity;
};

} } // namespaces
