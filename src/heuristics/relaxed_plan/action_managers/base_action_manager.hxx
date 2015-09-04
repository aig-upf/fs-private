
#pragma once

#include <fs0_types.hxx>

namespace fs0 {

class RPGData; class RelaxedState; class GroundAction;

//! Base interface for an action manager.
class BaseActionManager {
public:
	//!
	BaseActionManager() {};
	virtual ~BaseActionManager() {};

	//! Process the given action in the given layer of the RPG, adding newly-achievable atoms and their supoprts to the rpg data structure.
	virtual void process(unsigned actionIdx, const RelaxedState& layer, RPGData& rpg) = 0;
	
	virtual const GroundAction& getAction() const = 0;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const BaseActionManager& o) { return o.print(os); }
	virtual std::ostream& print(std::ostream& os) const = 0;
};


} // namespaces
