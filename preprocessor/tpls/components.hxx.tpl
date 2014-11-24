
#ifndef __APTK_CORE_EXAMPLES_COMPONENTS__
#define __APTK_CORE_EXAMPLES_COMPONENTS__

#include <cassert>
#include <cstring>
#include <stdexcept>

#include <state.hxx>
#include <actions.hxx>
#include <core_types.hxx>
#include <core_changeset.hxx>
#include <fact.hxx>
#include <unordered_set>
#include <unordered_map>



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


} } } // namespaces

#endif