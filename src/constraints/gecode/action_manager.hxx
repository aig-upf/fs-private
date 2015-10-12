
#pragma once

#include <fs0_types.hxx>
#include <constraints/gecode/handlers/csp_handler.hxx>
#include <actions/ground_action.hxx>

namespace fs0 { namespace gecode {
	
class GecodeRPGLayer;

//! An action manager based on modeling the action preconditions and effects as a CSP and solving it approximately / completely with Gecode.
class GecodeManager {
public:

	GecodeManager(unsigned action_idx, bool approximate)
		: _action_idx(action_idx), _approximate(approximate) {}
	
	virtual ~GecodeManager() { }

	//!
	virtual void process(const State& seed, const GecodeRPGLayer& layer, RPGData& rpg) const = 0;
	void process_handler(const State& seed, GecodeActionCSPHandler::ptr handler, const GecodeRPGLayer& layer, RPGData& rpg) const;

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const GecodeManager& o) { return o.print(os); }
	virtual std::ostream& print(std::ostream& os) const = 0;
	
	virtual const GroundAction& getAction() const = 0;
	
	
protected:
	//! The index of the action managed by this manager
	unsigned _action_idx;
	
	//! Whether to solve the action CSPs completely or approximately
	bool _approximate;
};

class GecodeActionManager : public GecodeManager {
public:
	//! Factory methods
	static std::vector<std::shared_ptr<GecodeManager>> create(const std::vector<GroundAction::cptr>& actions);

	GecodeActionManager(unsigned action_idx, bool approximate, GecodeActionCSPHandler::ptr handler)
		: GecodeManager(action_idx, approximate), _handler(handler) {}
	
	~GecodeActionManager() {
		delete _handler;
	}

	void process(const State& seed, const GecodeRPGLayer& layer, RPGData& rpg) const;

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os) const;
	
	const GroundAction& getAction() const { return _handler->getAction(); }
	
protected:
	//! The action handler that correspond to this action
	GecodeActionCSPHandler::ptr _handler;
};

class GecodeActionEffectManager : public GecodeManager {
public:
	//! Factory methods
	static std::vector<std::shared_ptr<GecodeManager>> create(const std::vector<GroundAction::cptr>& actions);

	GecodeActionEffectManager(unsigned action_idx, bool approximate, std::vector<GecodeActionCSPHandler::ptr>&& handlers)
		: GecodeManager(action_idx, approximate), _handlers(std::move(handlers)) {}
	
	~GecodeActionEffectManager() {
		for (auto handler:_handlers) delete handler;
	}

	//!
	void process(const State& seed, const GecodeRPGLayer& layer, RPGData& rpg) const;

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os) const;
	
	const GroundAction& getAction() const { return _handlers[0]->getAction(); }
	
	
protected:
	//! The set of (action / effect) handlers that correspond to this action
	std::vector<GecodeActionCSPHandler::ptr> _handlers;
};



} } // namespaces
