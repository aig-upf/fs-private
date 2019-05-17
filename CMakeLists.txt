cmake_minimum_required(VERSION 3.5)

project(lambda VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCE_FILES
        src/fs/core/actions/action_id
        src/fs/core/actions/actions
        src/fs/core/actions/checker
        src/fs/core/actions/grounding
        src/fs/core/applicability/action_managers
        src/fs/core/applicability/base.hxx
        src/fs/core/applicability/formula_interpreter
        src/fs/core/applicability/match_tree
        src/fs/core/constraints/registry
        src/fs/core/constraints/grounding
        src/fs/core/fstrips/fol
        src/fs/core/fstrips/language
        src/fs/core/fstrips/language_info
        src/fs/core/fstrips/problem
        src/fs/core/fstrips/interpretation
        src/fs/core/fstrips/loader
        src/fs/core/fstrips/operations
        src/fs/core/heuristics/novelty/features
        src/fs/core/heuristics/null_heuristic
        src/fs/core/heuristics/unsat_goal_atoms

        src/fs/core/languages/fstrips/operations/axioms
        src/fs/core/languages/fstrips/operations/basic
        src/fs/core/languages/fstrips/operations/binding
        src/fs/core/languages/fstrips/operations/conjunction
        src/fs/core/languages/fstrips/operations/interpretation
        src/fs/core/languages/fstrips/axioms
        src/fs/core/languages/fstrips/base
        src/fs/core/languages/fstrips/builtin
        src/fs/core/languages/fstrips/effects
        src/fs/core/languages/fstrips/factory
        src/fs/core/languages/fstrips/formulae
        src/fs/core/languages/fstrips/language.hxx
        src/fs/core/languages/fstrips/language_fwd.hxx
        src/fs/core/languages/fstrips/loader
        src/fs/core/languages/fstrips/operations.hxx
        src/fs/core/languages/fstrips/scopes
        src/fs/core/languages/fstrips/terms
        src/fs/core/search/nodes/blind_node.hxx
        src/fs/core/models/ground_state_model
        src/fs/core/models/simple_state_model
        src/fs/core/search/algorithms/iterated_width.hxx
        src/fs/core/search/drivers/base
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
        src/fs/core/search/drivers/validation.cxx
        src/fs/core/search/drivers/validation.hxx
        src/fs/core/search/nodes/bfws_node.hxx
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
        src/fs/core/atom.cxx
        src/fs/core/atom.hxx
        src/fs/core/base.cxx
        src/fs/core/base.hxx
        src/fs/core/fs_types
        src/fs/core/problem.cxx
        src/fs/core/problem.hxx
        src/fs/core/problem_info.cxx
        src/fs/core/problem_info.hxx
        src/fs/core/state.cxx
        src/fs/core/state.hxx
        src/fs/core/validator.cxx
        src/fs/core/validator.hxx


        src/fs/core/fstrips/grounding

        src/fs/core/lambda/states
        src/fs/core/lambda/search/factory
        src/fs/core/lambda/search/search_model
        src/fs/core/lambda/search/breadth_first_search


        pyfs/src/extension/fstrips
        pyfs/src/extension/language_info
        pyfs/src/extension/module
        pyfs/src/extension/utils
        pyfs/src/extension/factories
        pyfs/src/extension/search
        )

#add_executable(fs_private ${SOURCE_FILES})
add_library(fs SHARED ${SOURCE_FILES})

set(FS_ROOT .)
set(FS_SRC ${FS_ROOT}/src)
set(FS_VENDOR ${FS_ROOT}/vendor)

#MESSAGE(WARNING "Your FS_ROOT home directory is: ${FS_ROOT}" )
#MESSAGE(WARNING "Your FS_SRC home directory is: ${FS_SRC}" )
#MESSAGE(WARNING  "Your LAPKT directory is: $ENV{LAPKT}/include" )
#MESSAGE(WARNING "Your Unix home directory is: $ENV{HOME}" )


include_directories($ENV{HOME}/local/include)  # Gecode, etc.
include_directories(${FS_SRC})

include_directories(${FS_VENDOR}/rapidjson/include)
include_directories(${FS_VENDOR}/lapkt-base/src)
include_directories(${FS_VENDOR}/lapkt-novelty/src)
