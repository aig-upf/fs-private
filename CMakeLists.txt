cmake_minimum_required(VERSION 3.5)
project(fs_private)

project(fslib VERSION 0.1)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCE_FILES
        src/fs/core/actions/action_id.cxx
        src/fs/core/actions/action_id.hxx
        src/fs/core/actions/actions.cxx
        src/fs/core/actions/actions.hxx
        src/fs/core/actions/checker.cxx
        src/fs/core/actions/checker.hxx
        src/fs/core/actions/grounding.cxx
        src/fs/core/actions/grounding.hxx
        src/fs/core/actions/csp_action_iterator
        src/fs/core/actions/sdd_action_iterator
        src/fs/core/applicability/action_managers.cxx
        src/fs/core/applicability/action_managers.hxx
        src/fs/core/applicability/base.hxx
        src/fs/core/applicability/formula_interpreter.cxx
        src/fs/core/applicability/formula_interpreter.hxx
        src/fs/core/applicability/gecode_analyzer.cxx
        src/fs/core/applicability/gecode_analyzer.hxx
        src/fs/core/applicability/match_tree.cxx
        src/fs/core/applicability/match_tree.hxx
        src/fs/core/constraints/gecode/handlers/base_action_csp.cxx
        src/fs/core/constraints/gecode/handlers/base_action_csp.hxx
        src/fs/core/constraints/gecode/handlers/base_csp.cxx
        src/fs/core/constraints/gecode/handlers/base_csp.hxx
        src/fs/core/constraints/gecode/handlers/formula_csp.cxx
        src/fs/core/constraints/gecode/handlers/formula_csp.hxx
        src/fs/core/constraints/gecode/handlers/monotonicity_csp
        src/fs/core/constraints/gecode/handlers/ground_action_csp.cxx
        src/fs/core/constraints/gecode/handlers/ground_action_csp.hxx
        src/fs/core/constraints/gecode/handlers/ground_effect_csp.cxx
        src/fs/core/constraints/gecode/handlers/ground_effect_csp.hxx
        src/fs/core/constraints/gecode/handlers/lifted_action_csp.cxx
        src/fs/core/constraints/gecode/handlers/lifted_action_csp.hxx
        src/fs/core/constraints/gecode/handlers/lifted_effect_csp.cxx
        src/fs/core/constraints/gecode/handlers/lifted_effect_csp.hxx
        src/fs/core/constraints/gecode/handlers/lifted_effect_unreached.cxx
        src/fs/core/constraints/gecode/handlers/lifted_effect_unreached.hxx
        src/fs/core/constraints/gecode/language_translators.cxx
        src/fs/core/constraints/gecode/language_translators.hxx
        src/fs/core/constraints/gecode/utils/element_constraint.cxx
        src/fs/core/constraints/gecode/utils/element_constraint.hxx
        src/fs/core/constraints/gecode/utils/extensional_constraint.cxx
        src/fs/core/constraints/gecode/utils/extensional_constraint.hxx
        src/fs/core/constraints/gecode/utils/nested_fluent_iterator.cxx
        src/fs/core/constraints/gecode/utils/nested_fluent_iterator.hxx
        src/fs/core/constraints/gecode/utils/novelty_constraints.cxx
        src/fs/core/constraints/gecode/utils/novelty_constraints.hxx
        src/fs/core/constraints/gecode/utils/term_list_iterator.cxx
        src/fs/core/constraints/gecode/utils/term_list_iterator.hxx
        src/fs/core/constraints/gecode/utils/value_selection.cxx
        src/fs/core/constraints/gecode/utils/value_selection.hxx
        src/fs/core/constraints/gecode/csp_translator.cxx
        src/fs/core/constraints/gecode/csp_translator.hxx
        src/fs/core/constraints/gecode/extensions.cxx
        src/fs/core/constraints/gecode/extensions.hxx
        src/fs/core/constraints/gecode/gecode_space.cxx
        src/fs/core/constraints/gecode/gecode_space.hxx
        src/fs/core/constraints/gecode/helper.cxx
        src/fs/core/constraints/gecode/helper.hxx
        src/fs/core/constraints/gecode/lifted_plan_extractor.cxx
        src/fs/core/constraints/gecode/lifted_plan_extractor.hxx
        src/fs/core/constraints/gecode/supports.cxx
        src/fs/core/constraints/gecode/supports.hxx
        src/fs/core/constraints/filtering.hxx
        src/fs/core/constraints/registry.cxx
        src/fs/core/constraints/registry.hxx
        src/fs/core/constraints/native/action_handler
        src/fs/core/fstrips/language.cxx
        src/fs/core/fstrips/language.hxx
        src/fs/core/fstrips/language_info.cxx
        src/fs/core/fstrips/language_info.hxx
        src/fs/core/fstrips/loader.cxx
        src/fs/core/fstrips/loader.hxx
        src/fs/core/fstrips/operations.cxx
        src/fs/core/fstrips/operations.hxx
        src/fs/core/heuristics/novelty/features.cxx
        src/fs/core/heuristics/novelty/features.hxx
        src/fs/core/heuristics/relaxed_plan/gecode_crpg.cxx
        src/fs/core/heuristics/relaxed_plan/gecode_crpg.hxx
        src/fs/core/heuristics/relaxed_plan/relaxed_plan.cxx
        src/fs/core/heuristics/relaxed_plan/relaxed_plan.hxx
        src/fs/core/heuristics/relaxed_plan/relaxed_plan_extractor.hxx
        src/fs/core/heuristics/relaxed_plan/rpg_data.cxx
        src/fs/core/heuristics/relaxed_plan/rpg_data.hxx
        src/fs/core/heuristics/relaxed_plan/rpg_index.cxx
        src/fs/core/heuristics/relaxed_plan/rpg_index.hxx
        src/fs/core/heuristics/relaxed_plan/smart_rpg.cxx
        src/fs/core/heuristics/relaxed_plan/smart_rpg.hxx
        src/fs/core/heuristics/relaxed_plan/native_rpg
        src/fs/core/heuristics/null_heuristic.hxx
        src/fs/core/heuristics/unsat_goal_atoms.cxx
        src/fs/core/heuristics/unsat_goal_atoms.hxx
        src/fs/core/languages/fstrips/operations/axioms.cxx
        src/fs/core/languages/fstrips/operations/axioms.hxx
        src/fs/core/languages/fstrips/operations/basic.cxx
        src/fs/core/languages/fstrips/operations/basic.hxx
        src/fs/core/languages/fstrips/operations/binding.cxx
        src/fs/core/languages/fstrips/operations/binding.hxx
        src/fs/core/languages/fstrips/operations/conjunction.cxx
        src/fs/core/languages/fstrips/operations/conjunction.hxx
        src/fs/core/languages/fstrips/operations/interpretation.cxx
        src/fs/core/languages/fstrips/operations/interpretation.hxx
        src/fs/core/languages/fstrips/axioms.cxx
        src/fs/core/languages/fstrips/axioms.hxx
        src/fs/core/languages/fstrips/base.cxx
        src/fs/core/languages/fstrips/base.hxx
        src/fs/core/languages/fstrips/builtin.cxx
        src/fs/core/languages/fstrips/builtin.hxx
        src/fs/core/languages/fstrips/effects.cxx
        src/fs/core/languages/fstrips/effects.hxx
        src/fs/core/languages/fstrips/factory.cxx
        src/fs/core/languages/fstrips/factory.hxx
        src/fs/core/languages/fstrips/formulae.cxx
        src/fs/core/languages/fstrips/formulae.hxx
        src/fs/core/languages/fstrips/complex_existential_formula.hxx
        src/fs/core/languages/fstrips/language.hxx
        src/fs/core/languages/fstrips/language_fwd.hxx
        src/fs/core/languages/fstrips/loader.cxx
        src/fs/core/languages/fstrips/loader.hxx
        src/fs/core/languages/fstrips/operations.hxx
        src/fs/core/languages/fstrips/scopes.cxx
        src/fs/core/languages/fstrips/scopes.hxx
        src/fs/core/languages/fstrips/terms.cxx
        src/fs/core/languages/fstrips/terms.hxx
        src/fs/core/search/nodes/blind_node.hxx
        src/fs/core/models/ground_state_model.cxx
        src/fs/core/models/ground_state_model.hxx
        src/fs/core/models/csp_lifted_state_model.cxx
        src/fs/core/models/csp_lifted_state_model.hxx
        src/fs/core/models/sdd_lifted_state_model
        src/fs/core/models/utils
        src/fs/core/models/simple_state_model.cxx
        src/fs/core/models/simple_state_model.hxx
        src/fs/core/search/algorithms/monotonic_search
        src/fs/core/search/algorithms/ehc.hxx
        src/fs/core/search/algorithms/ehc_gbfs.hxx
        src/fs/core/search/algorithms/iterated_width.hxx
        src/fs/core/search/drivers/base.hxx
        src/fs/core/search/drivers/sbfws/features/features.cxx
        src/fs/core/search/drivers/sbfws/features/features.hxx
        src/fs/core/search/drivers/sbfws/base.cxx
        src/fs/core/search/drivers/sbfws/base.hxx
        src/fs/core/search/drivers/sbfws/config.cxx
        src/fs/core/search/drivers/sbfws/config.hxx
        src/fs/core/search/drivers/sbfws/iw_run_config.hxx
        src/fs/core/search/drivers/sbfws/iw_run_config.cxx
        src/fs/core/search/drivers/sbfws/iw_run.cxx
        src/fs/core/search/drivers/sbfws/iw_run.hxx
        src/fs/core/search/drivers/sbfws/relevant_atomset.hxx
        src/fs/core/search/drivers/sbfws/relevant_atoms.hxx
        src/fs/core/search/drivers/sbfws/relevant_atoms.cxx
        src/fs/core/search/drivers/sbfws/sbfws.cxx
        src/fs/core/search/drivers/sbfws/sbfws.hxx
        src/fs/core/search/drivers/sbfws/stats.cxx
        src/fs/core/search/drivers/sbfws/stats.hxx
        src/fs/core/search/drivers/breadth_first_search.cxx
        src/fs/core/search/drivers/breadth_first_search.hxx
        src/fs/core/search/drivers/iterated_width.cxx
        src/fs/core/search/drivers/iterated_width.hxx
        src/fs/core/search/drivers/registry.cxx
        src/fs/core/search/drivers/registry.hxx
        src/fs/core/search/drivers/setups.cxx
        src/fs/core/search/drivers/setups.hxx
        src/fs/core/search/drivers/smart_effect_driver.cxx
        src/fs/core/search/drivers/smart_effect_driver.hxx
        src/fs/core/search/drivers/smart_lifted_driver.cxx
        src/fs/core/search/drivers/smart_lifted_driver.hxx
        src/fs/core/search/drivers/native_action_driver
        src/fs/core/search/drivers/validation.cxx
        src/fs/core/search/drivers/validation.hxx
        src/fs/core/search/nodes/bfws_node.hxx
        src/fs/core/search/nodes/heuristic_search_node.hxx
        src/fs/core/search/nodes/monotonic_node
        src/fs/core/search/novelty/fs_novelty.cxx
        src/fs/core/search/novelty/fs_novelty.hxx
        src/fs/core/search/events.hxx
        src/fs/core/search/options.cxx
        src/fs/core/search/options.hxx
        src/fs/core/search/runner.cxx
        src/fs/core/search/runner.hxx
        src/fs/core/search/stats.hxx
        src/fs/core/search/utils.hxx
        src/fs/core/utils/printers/actions.cxx
        src/fs/core/utils/printers/actions.hxx
        src/fs/core/utils/printers/binding.cxx
        src/fs/core/utils/printers/binding.hxx
        src/fs/core/utils/printers/gecode.cxx
        src/fs/core/utils/printers/gecode.hxx
        src/fs/core/utils/printers/helper.cxx
        src/fs/core/utils/printers/helper.hxx
        src/fs/core/utils/printers/language.cxx
        src/fs/core/utils/printers/language.hxx
        src/fs/core/utils/printers/printers.cxx
        src/fs/core/utils/printers/printers.hxx
        src/fs/core/utils/printers/registry.cxx
        src/fs/core/utils/printers/registry.hxx
        src/fs/core/utils/printers/vector.hxx
        src/fs/core/utils/atom_index.cxx
        src/fs/core/utils/atom_index.hxx
        src/fs/core/utils/binding.cxx
        src/fs/core/utils/binding.hxx
        src/fs/core/utils/binding_iterator.cxx
        src/fs/core/utils/binding_iterator.hxx
        src/fs/core/utils/bitsets.hxx
        src/fs/core/utils/cartesian_iterator.cxx
        src/fs/core/utils/cartesian_iterator.hxx
        src/fs/core/utils/component_factory.hxx
        src/fs/core/utils/config.cxx
        src/fs/core/utils/config.hxx
        src/fs/core/utils/dispatcher.hxx
        src/fs/core/utils/external.hxx
        src/fs/core/utils/index.hxx
        src/fs/core/utils/lexical_cast.hxx
        src/fs/core/utils/loader.cxx
        src/fs/core/utils/loader.hxx
        src/fs/core/utils/projections.cxx
        src/fs/core/utils/projections.hxx
        src/fs/core/utils/serialize_tuple.hxx
        src/fs/core/utils/serializer.cxx
        src/fs/core/utils/serializer.hxx
        src/fs/core/utils/static.cxx
        src/fs/core/utils/static.hxx
        src/fs/core/utils/support.cxx
        src/fs/core/utils/support.hxx
        src/fs/core/utils/system.cxx
        src/fs/core/utils/system.hxx
        src/fs/core/utils/tuple_hash.hxx
        src/fs/core/utils/utils.hxx
        src/fs/core/utils/visitor.hxx
        src/fs/core/utils/sdd
        src/fs/core/atom.cxx
        src/fs/core/atom.hxx
        src/fs/core/base.cxx
        src/fs/core/base.hxx
        src/fs/core/fs_types
        src/fs/core/problem.cxx
        src/fs/core/problem.hxx
        src/fs/core/problem_info.cxx
        src/fs/core/problem_info.hxx
        src/fs/core/relaxed_state.cxx
        src/fs/core/relaxed_state.hxx
        src/fs/core/state.cxx
        src/fs/core/state.hxx
        src/fs/core/monotonicity
        src/fs/core/validator.cxx
        src/fs/core/validator.hxx
        src/fs/core/search/drivers/sbfws/simulation_evaluators.hxx)

#add_executable(fs_private ${SOURCE_FILES})
add_library(fs SHARED ${SOURCE_FILES})

set(FS_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(FS_SRC ${FS_ROOT}/src)
set(FS_VENDOR ${FS_ROOT}/vendor)

#MESSAGE(WARNING "Your FS_ROOT home directory is: ${FS_ROOT}" )
#MESSAGE(WARNING "Your FS_SRC home directory is: ${FS_SRC}" )
#MESSAGE(WARNING  "Your LAPKT directory is: $ENV{LAPKT}/include" )
#MESSAGE(WARNING "Your Unix home directory is: $ENV{HOME}" )
#MESSAGE(WARNING "Your directory is: ${FS_ROOT}" )
#MESSAGE(WARNING "Your directory is: ${FS_VENDOR}" )
#MESSAGE(WARNING "Your directory is: $ENV{HOME}/local/include" )


include_directories($ENV{HOME}/local/include)  # Gecode, etc.
include_directories(${FS_SRC})
include_directories(${FS_VENDOR}/rapidjson/include)
# MRJ: is this necessary? GFM: Yes, for the includes to <lapkt/*> to work
include_directories(${FS_VENDOR}/lapkt-base/src)
include_directories(${FS_VENDOR}/lapkt-novelty/src)
include_directories(${FS_VENDOR}/sdd)

#set(CMAKE_VERBOSE_MAKEFILE ON)
