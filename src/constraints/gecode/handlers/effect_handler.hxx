
#pragma once

#include <constraints/gecode/handlers/action_handler.hxx>

namespace fs0 { namespace gecode {

//! A CSP modeling and solving the effect of an action effect on a certain RPG layer
class GecodeEffectCSPHandler : public GecodeActionCSPHandler {
public:
	typedef GecodeEffectCSPHandler* ptr;
	typedef const GecodeEffectCSPHandler* cptr;

	GecodeEffectCSPHandler(const GroundAction& action, unsigned effect_idx, bool use_novelty_constraint);
	 
	~GecodeEffectCSPHandler() {}
};


} } // namespaces
