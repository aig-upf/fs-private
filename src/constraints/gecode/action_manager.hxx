
#pragma once

#include <fs0_types.hxx>
#include <heuristics/rpg/base_action_manager.hxx>
#include <constraints/gecode/handlers/csp_handler.hxx>

namespace fs0 { class RPGData; }

namespace fs0 { namespace gecode {

//! An action manager based on modeling the action preconditions and effects as a CSP and solving it approximately / completely with Gecode.
class GecodeActionManager : public BaseActionManager {
public:
	typedef GecodeActionManager* ptr;
	
	~GecodeActionManager() { delete _handler; }

	//!
	void process(unsigned actionIdx, const RelaxedState& layer, RPGData& rpg);

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os) const;
	
	//! Factory method - ownership of the pointer belongs to the caller
	static GecodeActionManager::ptr create(const GroundAction& action);
	
	const GroundAction& getAction() const;

protected:
	//! Private constructor, use factory method instead
	GecodeActionManager(GecodeActionCSPHandler::ptr handler) : _handler(handler) {}
	
	GecodeActionCSPHandler::ptr _handler;
};



} } // namespaces
