
#pragma once

#include <fs0_types.hxx>

namespace fs0 { namespace language { namespace fstrips { class Formula; class ActionEffect; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 {

//! A base interface for the different types of actions that the planners deals with:
//! lifted and grounded actions
class BaseAction {
protected:
	//! The id that identifies the concrete action within the set of actions of the same type
	//! (i.e. a grounded and a lifted action might have the same ID and yet be different entities)
	unsigned _id;
	
	//! The action preconditions  and effects
	const fs::Formula* _precondition;
	const std::vector<const fs::ActionEffect*> _effects;

public:
	typedef const BaseAction* cptr;
	
	BaseAction(unsigned id, const fs::Formula* precondition, const std::vector<const fs::ActionEffect*>& effects);
	virtual ~BaseAction();
	
	//! Returns the name of the action, e.g. 'move'
	virtual const std::string& getName() const = 0;
	
	unsigned getId() const { return _id; }
	
	//! Returns the full, grounded name of the action, e.g. 'move(b1, c2)'
	virtual std::string fullname() const = 0;
	
	//! Returns the signature of the action
	virtual const Signature& getSignature() const = 0;
	
	const fs::Formula* getPrecondition() const { return _precondition; }
	
	const std::vector<const fs::ActionEffect*>& getEffects() const { return _effects; }
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const BaseAction&  entity) { return entity.print(os); }
	virtual std::ostream& print(std::ostream& os) const;
};


} // namespaces
