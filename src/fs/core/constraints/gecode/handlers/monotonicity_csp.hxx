
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

    FSGecodeSpace* check_consistency(FSGecodeSpace *csp, bool stick_to_solution) const;

    static FSGecodeSpace* solve_csp(FSGecodeSpace* csp);

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

    FSGecodeSpace* instantiate_from_changeset(const DomainTracker& base_domains, const State& state) const;

    DomainTracker prune_domains(const FSGecodeSpace& csp, const DomainTracker& tracker) const;

    DomainTracker compute_root_domains(const State& root) const;




    DomainTracker post_monotonicity_csp_from_domains(const State& state,
                                                     const DomainTracker& domains,
                                                     bool stick_to_solution) const;
};

gecode::MonotonicityCSP*
build_monotonicity_csp(const Problem& problem, const Config& config);

} } // namespaces
