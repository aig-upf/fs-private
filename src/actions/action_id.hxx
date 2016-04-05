
#pragma once

#include <fs0_types.hxx>
#include <actions/actions.hxx>

namespace fs0 { namespace gecode { class LiftedActionIterator; }}

namespace fs0 {

class ActionID {
public:
	ActionID() = default;
	virtual ~ActionID() {}
	
	virtual unsigned id() const = 0;
	
	virtual bool operator==(const ActionID& rhs) const = 0;
	inline bool operator!=(const ActionID& rhs) const { return !this->operator==(rhs); }
	
	virtual std::size_t hash_code() const = 0;
	
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
	const BaseAction* _action;
	
	//! The indexes of the action binding.
	std::vector<ObjectIdx> _binding; // TODO This should be const, but then we cannot have assignment operator
	
public:
	//! Type aliases required for the lifted state model
	typedef LiftedActionID IdType;
	typedef gecode::LiftedActionIterator ApplicableSet;
	
	static const LiftedActionID invalid_action_id;
	
	//! Constructors
	LiftedActionID(const BaseAction* action, const std::vector<ObjectIdx>& binding);
	LiftedActionID(const BaseAction* action, const Binding& binding);
	LiftedActionID(const BaseAction* action, std::vector<ObjectIdx>&& binding);
	
	//! Default copy constructors and assignment operators
	LiftedActionID(const LiftedActionID& other) = default;
	LiftedActionID(LiftedActionID&& other) = default;
	LiftedActionID& operator=(const LiftedActionID& other) = default;
	LiftedActionID& operator=(LiftedActionID&& other) = default;
	
	unsigned id() const;

	//! Returns the concrete binding that created this action from its action schema
	const std::vector<ObjectIdx>& getBinding() const { return _binding; }
	
	bool operator==(const ActionID& rhs) const;
	
	std::size_t hash_code() const;
	
	//! Generates the ground action actually represented by this lifted ID
	GroundAction* generate() const;
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os) const;
};

//! A plain action ID is just the unsigned integer that identifies the action within the whole vector of grounded actions
class PlainActionID : public ActionID {
protected:
	const BaseAction* _action;
public:
	PlainActionID(const BaseAction* action) : _action(action) {}

	unsigned id() const;
	
	bool operator==(const ActionID& rhs) const;
	
	std::size_t hash_code() const;
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os) const;
};

typedef std::vector<const ActionID*> plan_t;

} // namespaces

// STD specialization
namespace std {
    template<> struct hash<fs0::ActionID> {
        std::size_t operator()(const fs0::ActionID& element) const { return element.hash_code(); }
    };

    template<> struct hash<const fs0::ActionID*> {
        std::size_t operator()(const fs0::ActionID* element) const { return hash<fs0::ActionID>()(*element); }
    };
}