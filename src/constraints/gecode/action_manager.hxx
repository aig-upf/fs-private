
#pragma once

#include <fs0_types.hxx>
#include <constraints/gecode/handlers/csp_handler.hxx>
#include <actions/ground_action.hxx>

namespace fs0 { namespace gecode {
	
class GecodeRPGLayer;

//! An action manager based on modeling the action preconditions and effects as a CSP and solving it approximately / completely with Gecode.
class GecodeActionManager {
public:
	typedef GecodeActionManager* ptr;
	
	//! Factory methods
	static std::vector<std::shared_ptr<GecodeActionManager>> createActionCSPs(const std::vector<GroundAction::cptr>& actions);
	static std::vector<std::shared_ptr<GecodeActionManager>> createEffectCSPs(const std::vector<GroundAction::cptr>& actions);

	GecodeActionManager(std::vector<GecodeActionCSPHandler::ptr>&& handlers) : _handlers(std::move(handlers)) {}
	~GecodeActionManager() {
		for (auto handler:_handlers) delete handler;
	}

	//!
	void process(unsigned int actionIdx, const fs0::gecode::GecodeRPGLayer& layer, fs0::RPGData& rpg) const;
	void process_handler(GecodeActionCSPHandler::ptr handler, unsigned int actionIdx, const GecodeRPGLayer& layer, fs0::RPGData& rpg) const;

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const GecodeActionManager& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	
	const GroundAction& getAction() const { return _handlers[0]->getAction(); }
	
	
protected:
	std::vector<GecodeActionCSPHandler::ptr> _handlers;
};



} } // namespaces
