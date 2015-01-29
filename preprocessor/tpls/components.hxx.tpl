
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
#include <atoms.hxx>
#include <problem.hxx>
#include "external.hxx"
#include <constraints/all.hxx>

using namespace fs0;

extern std::unique_ptr<External> external;

/*********************************************/
/* The actions                               */
/*********************************************/
$action_definitions

/*********************************************/
/* Constraint and Effect components          */
/*********************************************/
${component_classes}

/*********************************************/
/* Method factories                          */
/*********************************************/
$method_factories

/* Generate the whole planning problem */
void generate(const std::string& data_dir, Problem& problem);
