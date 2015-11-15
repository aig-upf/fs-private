
#pragma once

#include <fs0_types.hxx>
#include <constraints/gecode/handlers/action_handler.hxx>

namespace fs0 { class State; class GroundAction; class RPGData; }
	
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
	

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const GecodeManager& o) { return o.print(os); }
	virtual std::ostream& print(std::ostream& os) const = 0;
	
	//! Initialize the value selector of the underlying CSPs
	virtual void init_value_selector(const RPGData* bookkeeping) = 0;
	
	
protected:
	//! The index of the action managed by this manager
	unsigned _action_idx;
	
	//! Whether to solve the action CSPs completely or approximately
	bool _approximate;
	
	//! A helper to processes a single CSP handler
	void process_handler(const State& seed, GecodeActionCSPHandler* handler, const GecodeRPGLayer& layer, RPGData& rpg) const;
};

//! A RPG action manager that deals with the one-csp-per-action paradigm
class GecodeActionManager : public GecodeManager {
public:
	//! Factory method
	static std::vector<std::shared_ptr<GecodeManager>> create(const std::vector<const GroundAction*>& actions);

	GecodeActionManager(unsigned action_idx, bool approximate, GecodeActionCSPHandler* handler)
		: GecodeManager(action_idx, approximate), _handler(handler) {}
	
	~GecodeActionManager();

	void process(const State& seed, const GecodeRPGLayer& layer, RPGData& rpg) const;

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os) const;
	
	const GroundAction& getAction() const;
	
	//! Initialize the value selector of the underlying CSPs
	void init_value_selector(const RPGData* bookkeeping);
	
protected:
	//! The action handler that correspond to this action
	GecodeActionCSPHandler* _handler;
};



//! A RPG action manager that deals with the one-csp-per-action-schema paradigm
class GecodeActionSchemaManager : public GecodeManager {
public:
	//! Factory method
	static std::vector<std::shared_ptr<GecodeManager>> create(const std::vector<ActionSchema::cptr>& actions);

	GecodeActionManager(unsigned schema_idx, bool approximate, GecodeActionCSPHandler::ptr handler)
		: GecodeManager(schema_idx, approximate), _handler(handler) {}
	
	~GecodeActionManager() {
		delete _handler;
	}

	void process(const State& seed, const GecodeRPGLayer& layer, RPGData& rpg) const;

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os) const;
	
	//! Initialize the value selector of the underlying CSPs
	void init_value_selector(const RPGData* bookkeeping) {
		_handler->init(bookkeeping);
	}
	
protected:
	//! The action handler that correspond to this action
	GecodeActionCSPHandler::ptr _handler;
};


//! A RPG action manager that deals with the one-csp-per-effect paradigm
class GecodeActionEffectManager : public GecodeManager {
public:
	//! Factory method
	static std::vector<std::shared_ptr<GecodeManager>> create(const std::vector<const GroundAction*>& actions);

	GecodeActionEffectManager(unsigned action_idx, bool approximate, std::vector<GecodeActionCSPHandler*>&& handlers)
		: GecodeManager(action_idx, approximate), _handlers(std::move(handlers)) {}
	
	~GecodeActionEffectManager();

	//!
	void process(const State& seed, const GecodeRPGLayer& layer, RPGData& rpg) const;

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os) const;
	
	const GroundAction& getAction() const;
	
	//! Initialize the value selector of the underlying CSPs
	void init_value_selector(const RPGData* bookkeeping);
	
protected:
	//! The set of (action / effect) handlers that correspond to this action
	std::vector<GecodeActionCSPHandler*> _handlers;
};



} } // namespaces
