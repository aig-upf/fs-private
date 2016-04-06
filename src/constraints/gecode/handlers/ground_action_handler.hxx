
#pragma once

#include <constraints/gecode/handlers/base_action_handler.hxx>

namespace fs0 {
class GroundAction;
}

namespace fs0 { namespace gecode {


//! A CSP modeling and solving the effect of an action on a certain RPG layer
class GroundActionCSPHandler : public BaseActionCSPHandler {
public:
	typedef GroundActionCSPHandler* ptr;
	
	//! Factory method
	static std::vector<std::shared_ptr<BaseActionCSPHandler>> create(const std::vector<const GroundAction*>& actions, const TupleIndex& tuple_index, bool approximate, bool novelty);

	//! Constructors / Destructor
	GroundActionCSPHandler(const GroundAction& action, const TupleIndex& tuple_index, bool approximate);
	virtual ~GroundActionCSPHandler() {}

protected:

	const ActionID* get_action_id(const SimpleCSP* solution) const;
	
	//! Log some handler-related into
	virtual void log() const;
};

} } // namespaces
