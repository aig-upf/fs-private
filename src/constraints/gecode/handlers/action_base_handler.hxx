
#pragma once

#include <constraints/gecode/handlers/base_handler.hxx>

namespace fs0 { namespace gecode {


//! A CSP modeling and solving the effect of an action on a certain RPG layer
class BaseActionCSPHandler : public BaseCSPHandler {
public:
	typedef BaseActionCSPHandler* ptr;

	//!
	virtual ~BaseActionCSPHandler() {}

	
};

} } // namespaces
