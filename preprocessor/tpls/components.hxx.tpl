
#pragma once

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <unordered_set>
#include <unordered_map>

#include <state.hxx>
#include <actions.hxx>
#include <core_types.hxx>
#include <heuristics/changeset.hxx>
#include <fact.hxx>
#include <core_problem.hxx>
#include <utils/loader.hxx>

using namespace aptk::core;

namespace aptk { namespace core { namespace solver {


/*********************************************/
/* The static data                           */
/*********************************************/
class Ext {
protected:
    ${data_declarations}

public:
    ${data_accessors}

    ${external_methods}
};

/*********************************************/
/* The actions                               */
/*********************************************/
$action_definitions

/*********************************************/
/* The goal evaluator                        */
/*********************************************/
$goal_evaluator_definition


/*********************************************/
/* Method factories                          */
/*********************************************/
$method_factories

/* Generate the whole planning problem */
void generate(const std::string& data_dir, aptk::core::Problem& problem);

} } } // namespaces
