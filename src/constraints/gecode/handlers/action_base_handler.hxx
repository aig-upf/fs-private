
#pragma once

#include <constraints/gecode/handlers/base_handler.hxx>

namespace fs0 { class BaseAction; }

namespace fs0 { namespace gecode {


//! A CSP modeling and solving the effect of an action on a certain RPG layer
class BaseActionCSPHandler : public BaseCSPHandler {
public:
	typedef BaseActionCSPHandler* ptr;

	BaseActionCSPHandler(const BaseAction& action, bool approximate);
	
	//!
	virtual ~BaseActionCSPHandler() {}
	
	//!
	virtual void process(const State& seed, const GecodeRPGLayer& layer, RPGData& rpg) const;

	//! Initialize the value selector of the underlying CSPs
	virtual void init_value_selector(const RPGData* bookkeeping) { init(bookkeeping);} // TODO - No need to have two different names for the same method
	
protected:
	//! The index of the action managed by this manager
	const BaseAction& _action;
	
	virtual void compute_support(SimpleCSP* csp, RPGData& rpg, const State& seed) const = 0;
	
	//! A simple helper to log the processing message
	virtual void log() const = 0;
};

} } // namespaces
