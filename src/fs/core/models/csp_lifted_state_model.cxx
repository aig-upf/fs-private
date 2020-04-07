
#include <fs/core/models/csp_lifted_state_model.hxx>

#include <fs/core/actions/actions.hxx>
#include <fs/core/actions/action_id.hxx>
#include <fs/core/applicability/formula_interpreter.hxx>
#include <fs/core/constraints/gecode/v2/action_schema_csp.hxx>
#include <fs/core/languages/fstrips/language.hxx>
#include <fs/core/models/utils.hxx>
#include <fs/core/problem.hxx>
#include <fs/core/state.hxx>
#include <fs/core/utils/system.hxx>

#include <boost/filesystem.hpp>   // includes all needed Boost.Filesystem declarations

#include <utility>


namespace fsys = boost::filesystem;

namespace fs0 {

CSPLiftedStateModel::CSPLiftedStateModel(
        const Problem& problem,
        std::vector<const fs::Formula*> subgoals,
        std::vector<const PartiallyGroundedAction*>&& schemas,
        std::vector<SimpleLiftedOperator>&& lifted_operators,
        std::vector<gecode::v2::ActionSchemaCSP>&& schema_csps,
        gecode::v2::SymbolExtensionGenerator&& extension_generator) :

    problem(problem),
    _subgoals(std::move(subgoals)),
    schemas(std::move(schemas)),
    lifted_operators(std::move(lifted_operators)),
    schema_csps(std::move(schema_csps)),
    extension_generator(std::move(extension_generator))
{}

CSPLiftedStateModel::~CSPLiftedStateModel() = default;


State CSPLiftedStateModel::init() const {
    // We need to make a copy so that we can return it as non-const.
    // Ugly, but this way we make it fit the search engine interface without further changes,
    // and this is only called once per search.
    return State(problem.getInitialState());
}

bool CSPLiftedStateModel::goal(const State& state) const {
    return problem.getGoalSatManager().satisfied(state);
}


State CSPLiftedStateModel::next(const State& state, const LiftedActionID& aid) const {
    auto& adata = aid.getActionData();
    auto& op = lifted_operators[adata.getId()];
    // Note that we don't need to check the precondition of the operator, only evaluate the effects:
    evaluate_simple_lifted_operator(state, op, aid.get_binding(), ProblemInfo::getInstance(), false, _effects_cache);
    return State(state, _effects_cache); // Copy everything into the new state and apply the changeset
}

gecode::CSPActionIterator CSPLiftedStateModel::applicable_actions(const State& state, bool enforce_state_constraints) const {
    // TODO At the moment we don't support state constraints anymore
    return gecode::CSPActionIterator(state, schema_csps, extension_generator.instantiate(state), schemas);
}


bool
CSPLiftedStateModel::goal(const StateT& s, unsigned i) const {
    Binding binding;
    return _subgoals.at(i)->interpret(s, binding);
// 	return s.contains(_subgoals.at(i)); // TODO SHOULD BE:
    // const Atom& subgoal = _subgoals.at(i);
    // return s.check(subgoal.getVariable(), s.getValue());
}

CSPLiftedStateModel
CSPLiftedStateModel::build(const Problem& problem, const ProblemInfo& info, const AtomIndex& atom_index) {
    fsys::path path(info.getDataDir() + "/csps");
    if (!fsys::exists(path)) {
        std::cerr << "Non-existing CSP directory: " << path << std::endl;
        exit_with(ExitCode::SEARCH_INPUT_ERROR);
    }

    // We store the correspondence between schemas and consistent CSPs, and between schemas and simplified operators
    std::vector<const PartiallyGroundedAction*> schemas;
    std::vector<gecode::v2::ActionSchemaCSP> csps, csps_tmp;
    std::vector<SimpleLiftedOperator> ops;

    // We use a vector of unsigned as a bit-vector, for performance reasons:
    std::vector<unsigned> symbols_in_extensions(info.getNumLogicalSymbols(), 0);

    const auto& original_schemas = problem.getPartiallyGroundedActions();

    // We'll proceed in two passes to be able to compute the set `symbols_in_extensions` before invoking the
    // `initialize()` method on the generated CSPs
    for (const auto& schema:original_schemas) {
        // Each action schema has a number of filenames starting with the name of the schema
        const std::string& schema_name = schema->getName();
        std::string fname = schema_name + ".csp";
        std::ifstream ifs(info.getDataDir() + "/csps/" + schema_name + ".csp");
        csps_tmp.push_back(gecode::v2::ActionSchemaCSP::load(ifs, info, symbols_in_extensions));
    }

    gecode::v2::SymbolExtensionGenerator extension_generator(info, atom_index, symbols_in_extensions);

    for (unsigned i=0; i < original_schemas.size(); ++i) {
        if (csps_tmp[i].initialize(extension_generator)) {
            csps.push_back(std::move(csps_tmp[i]));
            schemas.push_back(original_schemas[i]);
            ops.emplace_back(compile_schema_to_simple_lifted_operator(*original_schemas[i]));

        } else {
            // Note that here in the case where the CSP has been detected as inconsistent at preprocessing, we need to
            // add some dummy operator, so that the mapping between action schema IDs (which include those inconsistent)
            // and operators stays correct.
            ops.emplace_back();
        }
    }

    return CSPLiftedStateModel(
            problem,
            obtain_goal_atoms(problem.getGoalConditions()),
            std::move(schemas),
            std::move(ops),
            std::move(csps),
            std::move(extension_generator));
}

} // namespaces
