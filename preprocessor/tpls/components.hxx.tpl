
#pragma once

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <unordered_set>
#include <unordered_map>

#include <state.hxx>
#include <actions.hxx>
#include <fs0_types.hxx>
#include <heuristics/changeset.hxx>
#include <fact.hxx>
#include <problem.hxx>
#include "external.hxx"

using namespace fs0;

extern std::unique_ptr<External> external;

/*********************************************/
/* The actions                               */
/*********************************************/
$action_definitions

/*********************************************/
/* The goal evaluator                        */
/*********************************************/
$goal_evaluator_definition

${goal_constraints}

/*********************************************/
/* Method factories                          */
/*********************************************/
$method_factories

/* Generate the whole planning problem */
void generate(const std::string& data_dir, Problem& problem);
