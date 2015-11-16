
#pragma once

#include <fs0_types.hxx>
#include <actions/action_schema.hxx>

namespace fs0 {

class ActionID {
protected:
	//! The id of the grounded action or action schema
	unsigned _id;
	
public:
	//! Factory method - returns a canonical invalid ActionID object
	static const ActionID* make_invalid();

	ActionID(unsigned id) : _id(id) {}
	virtual ~ActionID() {}
	
	bool is_valid() const { return _id != std::numeric_limits<unsigned int>::max(); }
	
	virtual bool operator==(const ActionID& rhs) const = 0;
	virtual bool operator< (const ActionID& rhs) const = 0;

	inline bool operator!=(const ActionID& rhs) const { return !this->operator==(rhs); }
	inline bool operator> (const ActionID& rhs) const {return  rhs.operator<(*this);}
	inline bool operator<=(const ActionID& rhs) const {return !this->operator>(rhs);}
	inline bool operator>=(const ActionID& rhs) const {return !this->operator<(rhs);}
	
	virtual std::size_t hash_code() const = 0;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ActionID&  entity) { return entity.print(os); }
	virtual std::ostream& print(std::ostream& os) const = 0;
};

//! An action is fully identified by the ID of the action schema and the values of its parameters,
//! i.e. its binding
class LiftedActionID : public ActionID  {
protected:
	//! The indexes of the action binding.
	const std::vector<ObjectIdx> _binding;
	
public:
	LiftedActionID(unsigned schema_id, const std::vector<ObjectIdx>& binding);
	LiftedActionID(unsigned schema_id, const Binding& binding);
	LiftedActionID(unsigned schema_id, std::vector<ObjectIdx>&& binding);
	
	bool operator==(const ActionID& rhs) const;
	bool operator< (const ActionID& rhs) const;
	
	std::size_t hash_code() const;
	
	//! Returns the concrete binding that created this action from its action schema
	const std::vector<ObjectIdx>& getBinding() const { return _binding; }
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os) const;
};

//! A plain action ID is just the unsigned integer that identifies the action within the whole vector of grounded actions
class PlainActionID : public ActionID {
public:
	PlainActionID(unsigned id) : ActionID(id) {}
	
	bool operator==(const ActionID& rhs) const;
	bool operator< (const ActionID& rhs) const;
	
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