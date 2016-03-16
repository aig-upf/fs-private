
#pragma once

#include <constraints/gecode/handlers/base_action_handler.hxx>

namespace fs0 { namespace gecode {

//! A CSP modeling and solving the effect of an action effect on a certain RPG layer
class GroundEffectCSPHandler : public BaseActionCSPHandler {
public:
	typedef GroundEffectCSPHandler* ptr;
	typedef const GroundEffectCSPHandler* cptr;
	
	//! Factory method
	static std::vector<std::shared_ptr<BaseActionCSPHandler>> create(const std::vector<const GroundAction*>& actions, bool approximate, bool novelty);

	GroundEffectCSPHandler(const GroundAction& action, unsigned effect_idx, bool approximate, bool novelty);
	 
	~GroundEffectCSPHandler() {}
	
protected:
	const ActionID* get_action_id(SimpleCSP* solution) const;

	void log() const;
};


} } // namespaces
