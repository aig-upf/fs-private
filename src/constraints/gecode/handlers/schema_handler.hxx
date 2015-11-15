
#pragma once

#include <constraints/gecode/handlers/action_handler.hxx>
#include <actions/action_schema.hxx>

namespace fs0 { namespace gecode {


//! A CSP modeling and solving the effect of an action on a certain RPG layer
class GecodeSchemaCSPHandler : public GecodeElementCSPHandler<ActionSchema> {
public:
	typedef GecodeSchemaCSPHandler* ptr;

protected:

};

} } // namespaces
