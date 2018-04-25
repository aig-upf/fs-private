
#pragma once

#include <fs/core/constraints/gecode/handlers/formula_csp.hxx>
#include <fs/core/monotonicity.hxx>

namespace fs0 { class AtomIndex; class Problem; class Config; }
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


    GecodeCSP* check_consistency(GecodeCSP *csp) const;

    GecodeCSP* check_consistency_from_changeset(const GecodeCSP& parent_csp, const State& state, const std::vector<Atom>& changeset) const;

    static GecodeCSP* solve_csp(GecodeCSP* csp);

    DomainTracker create_root(const State& root) const;


    DomainTracker
    generate_node(const State& parent, const DomainTracker& parent_domains, const State& child,
                  const std::vector<Atom>& changeset) const;

protected:
	const TransitionGraph _monotonicity;

    bool check_transitions(const State& parent,
                           const std::vector<Atom>& changeset) const;


    DomainTracker
    compute_base_domains(const DomainTracker& parent_domains, const std::vector<Atom>& changeset) const;

    GecodeCSP* instantiate_from_changeset_NEW(const DomainTracker& base_domains, const State& state) const;

    DomainTracker prune_domains(const GecodeCSP& csp, const DomainTracker& tracker) const;

    DomainTracker compute_root_domains(const State& root) const;




    DomainTracker post_monotonicity_csp_from_domains(const State& state,
                                                     const DomainTracker& domains) const;
};

gecode::MonotonicityCSP*
build_monotonicity_csp(const Problem& problem, const Config& config);

} } // namespaces
