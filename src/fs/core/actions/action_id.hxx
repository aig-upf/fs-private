
#pragma once

#include <fs/core/fs_types.hxx>
#include <fs/core/utils/binding.hxx>

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