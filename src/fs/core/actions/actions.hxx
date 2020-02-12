
#pragma once

#include <fs/core/fs_types.hxx>
#include <fs/core/utils/binding.hxx>
#include <fs/core/applicability/action_managers.hxx>
#include <fs/core/languages/fstrips/axioms.hxx>
#include <utility>

namespace fs0::language::fstrips {  class Term; class Formula; class ActionEffect; class ProceduralEffect; }
namespace fs = fs0::language::fstrips;

namespace fs0 {

class ProblemInfo;
class Atom;
class State;

//! All the data that fully characterizes a lifted action
class ActionData {
public:

        //! Action types (for a rationale of this taxonomy see Reiter R.,
        //! "Knowledge in Action", MIT Press, 2001):
        //!
        //! - Control : actions as in PDDL :action
        //! - Exogenous : exogenous actions as in PDDL+ :event
        //! - Natural: continuous exogenous actions as in PDDL+ :process
        enum class Type : char {
            Control,
            Exogenous,
            Natural
        };

protected:
	//! The ID of the original action schema (not to be confused with the ID of resulting fully-grounded actions)
	unsigned _id;

	const std::string                              _name;
	const Signature                                _signature;
	const std::vector<std::string>                 _parameter_names;
	fs::BindingUnit                                _bunit;
	const fs::Formula*                             _precondition;
	const std::vector<const fs::ActionEffect*>     _effects;
    Type                                           _type;

public:
	ActionData(unsigned id, const std::string& name, const Signature& signature, const std::vector<std::string>& parameter_names, const fs::BindingUnit& bunit,
			   const fs::Formula* precondition, const std::vector<const fs::ActionEffect*>& effects, Type t );

	~ActionData();
	ActionData(const ActionData&);

	unsigned getId() const { return _id; }
    bool hasProceduralEffects() const { return _name[0] == '@'; }
	const std::string& getName() const { return _name; }
	const Signature& getSignature() const { return _signature; }
	const std::vector<std::string>& getParameterNames() const { return _parameter_names; }
	const fs::BindingUnit& getBindingUnit() const { return _bunit; }
	const fs::Formula* getPrecondition() const { return _precondition; }
	const std::vector<const fs::ActionEffect*>& getEffects() const { return _effects; }
    Type getType() const { return _type; }

	//! Returns true if some parameter of the action has a type with no associated object
	bool has_empty_parameter() const;

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ActionData& entity) { return entity.print(os); }
	std::ostream& print(std::ostream& os) const;
};


//! A base interface for the different types of actions that the planners deals with:
//! lifted and grounded actions
class ActionBase {
protected:
	//! The data of the action schema that has originated this action
	const ActionData& _data;

	//! The action binding (which will be an empty binding for fully-lifted actions,
	//! and a full binding for fully-grounded actions, all steps in between being possible).
	const Binding _binding;

	//! The action preconditions and effects, perhaps partially grounded
	const fs::Formula* _precondition;
	const std::vector<const fs::ActionEffect*> _effects;

public:

	ActionBase(const ActionData& action_data, const Binding& binding, const fs::Formula* precondition, const std::vector<const fs::ActionEffect*>& effects);
	virtual ~ActionBase();

	ActionBase(const ActionBase& o);

	const ActionData& getActionData() const { return _data; };

	//! Some method redirections
	unsigned getOriginId() const { return _data.getId(); }
	const std::string& getName() const { return _data.getName(); }
    bool hasProceduralEffects() const { return _data.hasProceduralEffects(); }
	const Signature& getSignature() const { return _data.getSignature(); }
	const std::vector<std::string>& getParameterNames() const { return _data.getParameterNames(); }
	const fs::BindingUnit& getBindingUnit() const { return _data.getBindingUnit(); }
	unsigned numParameters() const { return getSignature().size(); }

	const fs::Formula* getPrecondition() const { return _precondition; }
	const std::vector<const fs::ActionEffect*>& getEffects() const { return _effects; }

	//!
	const Binding& getBinding() const { return _binding; }

	//! Return true iff the i-th action parameter is bound
	bool isBound(unsigned i) const { return _binding.binds(i); }

	//! Returns true if some parameter of the action has a type with no associated object
	bool has_empty_parameter() const { return _data.has_empty_parameter(); }

    //! Type of action accessors
    bool    isControl() const { return _data.getType() == ActionData::Type::Control; }
    bool    isExogenous() const { return _data.getType() == ActionData::Type::Exogenous; }
    bool    isNatural() const { return _data.getType() == ActionData::Type::Natural; }


    //! Apply action effects
    //! NOTE: clients need to clear the atoms vector
    virtual void apply( const State& s, std::vector<Atom>& atoms ) const = 0;

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ActionBase&  entity) { return entity.print(os); }
	virtual std::ostream& print(std::ostream& os) const;
};

//! An action that can be partially grounded (or fully lifted)
class PartiallyGroundedAction : public ActionBase {
public:
	PartiallyGroundedAction(const ActionData& action_data, const Binding& binding, const fs::Formula* precondition, const std::vector<const fs::ActionEffect*>& effects);
	~PartiallyGroundedAction() override = default;
	PartiallyGroundedAction(const PartiallyGroundedAction&) = default;

    void apply( const State& s, std::vector<Atom>& atoms ) const override;
};


//! A fully-grounded action can get an integer ID for more performant lookups
class GroundAction : public ActionBase {
protected:
	//! The id that identifies the concrete action within the whole set of ground actions
	unsigned _id;

public:
	//! Trait required by aptk::DetStateModel
	using IdType = ActionIdx;
	using ApplicableSet = GroundApplicableSet;

	static const ActionIdx invalid_action_id;

	GroundAction(unsigned id, const ActionData& action_data, const Binding& binding, const fs::Formula* precondition, const std::vector<const fs::ActionEffect*>& effects);
	~GroundAction() override = default;
	GroundAction(const GroundAction&) = default;

	unsigned getId() const { return _id; }
    void apply( const State& s, std::vector<Atom>& atoms ) const override;

};

class ProceduralAction : public GroundAction {
protected:

    fs::ProceduralEffect*     _proc_effect;

public:
    ProceduralAction( unsigned id, const ActionData& action_data, const Binding& binding, const fs::Formula* precondition, const std::vector<const fs::ActionEffect*>& effects);
    ProceduralAction( unsigned id, const ActionData& action_data, const Binding& binding);

    void apply( const State& s, std::vector<Atom>& atoms ) const override;

    ~ProceduralAction() override;
};

class PlainOperator {
public:
    PlainOperator(std::vector<std::pair<VariableIdx, object_id>> precondition,
                  std::vector<std::pair<VariableIdx, object_id>> effects) :
            precondition_(std::move(precondition)), effects_(std::move(effects))
    {}

    std::vector<std::pair<VariableIdx, object_id>> precondition_;
    std::vector<std::pair<VariableIdx, object_id>> effects_;
};

PlainOperator compile_action_to_plan_operator(const GroundAction& action);


} // namespaces
