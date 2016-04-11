
#pragma once

#include <fs0_types.hxx>
#include <utils/binding.hxx>

namespace fs0 { namespace gecode { class LiftedActionIterator; }}

namespace fs0 {

class PartiallyGroundedAction;
class GroundAction;

class ActionID {
public:
	ActionID() = default;
	virtual ~ActionID() {}
	
	virtual bool operator==(const ActionID& rhs) const = 0;
	inline bool operator!=(const ActionID& rhs) const { return !this->operator==(rhs); }
	
	virtual std::size_t hash() const = 0;
	
	//! Default copy constructors and assignment operators
	ActionID(const ActionID& other) = default;
	ActionID(ActionID&& other) = default;
	ActionID& operator=(const ActionID& other) = default;
	ActionID& operator=(ActionID&& other) = default;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ActionID&  entity) { return entity.print(os); }
	virtual std::ostream& print(std::ostream& os) const = 0;
};

//! An action is fully identified by the ID of the action schema and the values of its parameters,
//! i.e. its binding
class LiftedActionID : public ActionID  {
protected:
	//! The id of the grounded action or action schema
	const PartiallyGroundedAction* _action;
	
	//! The indexes of the action binding.
	Binding _binding; // TODO This should be const, but then we cannot have assignment operator
	
	//! The hash code of the object
	mutable std::size_t _hash;
	
	//! Whether the object has already been hashed or not (necessary to implement lazy hashing)
	mutable bool _hashed;
	
public:
	//! Type aliases required for the lifted state model
	typedef LiftedActionID IdType;
	typedef gecode::LiftedActionIterator ApplicableSet;
	
	static const LiftedActionID invalid_action_id;
	
	//! Constructors
	LiftedActionID(const PartiallyGroundedAction* action, Binding&& binding);
	
	//! Default copy constructors and assignment operators
	LiftedActionID(const LiftedActionID& other) = default;
	LiftedActionID(LiftedActionID&& other) = default;
	LiftedActionID& operator=(const LiftedActionID& other) = default;
	LiftedActionID& operator=(LiftedActionID&& other) = default;
	
	bool operator==(const ActionID& rhs) const;
	
	//! Hash-related operations
	std::size_t generate_hash() const;
	std::size_t hash() const;
	
	//! Generates the ground action actually represented by this lifted ID
	GroundAction* generate() const;
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os) const;

protected:
	Binding get_full_binding() const;
};

//! A plain action ID is just the unsigned integer that identifies the action within the whole vector of grounded actions
class PlainActionID : public ActionID {
protected:
	const GroundAction* _action;
public:
	PlainActionID(const GroundAction* action) : _action(action) {}

	unsigned id() const;
	
	bool operator==(const ActionID& rhs) const;
	
	std::size_t hash() const;
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os) const;
};

typedef std::vector<const ActionID*> plan_t;

} // namespaces

// STD specialization
namespace std {
    template<> struct hash<fs0::ActionID> {
        std::size_t operator()(const fs0::ActionID& element) const { return element.hash(); }
    };

    template<> struct hash<const fs0::ActionID*> {
        std::size_t operator()(const fs0::ActionID* element) const { return hash<fs0::ActionID>()(*element); }
    };
}