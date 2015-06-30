
#pragma once

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <unordered_set>
#include <unordered_map>

#include <state.hxx>
#include <actions.hxx>
#include <fs0_types.hxx>
#include <atoms.hxx>
#include <problem.hxx>
#include "external.hxx"
#include <constraints/all.hxx>
#include <gecode/int.hh>
#include <constraints/gecode/expr_translators.hxx>
#include <relaxed_action_manager.hxx>

using namespace fs0;

extern std::unique_ptr<External> external;

/*********************************************/
/* The actions                               */
/*********************************************/

class IncrementAction : public Action {

protected:
    static const std::string name_;
    static const ActionSignature signature_;

public:
	IncrementAction(const ObjectIdxVector& binding, const ScopedConstraint::vcptr& constraints, const ScopedEffect::vcptr& effects) : Action(binding, constraints, effects) {}

    const std::string& getName() const { return name_; }
    const ActionSignature& getSignature() const { return signature_; }
};

class DecrementAction : public Action {

protected:
    static const std::string name_;
    static const ActionSignature signature_;

public:
	DecrementAction(const ObjectIdxVector& binding, const ScopedConstraint::vcptr& constraints, const ScopedEffect::vcptr& effects) : Action(binding, constraints, effects) {}

    const std::string& getName() const { return name_; }
    const ActionSignature& getSignature() const { return signature_; }
};

/*********************************************/
/* Constraint and Effect components          */
/*********************************************/





class IncrementActionEffect0 : public UnaryScopedEffect {
public:
	IncrementActionEffect0(const VariableIdxVector& scope, const VariableIdxVector& image, const std::vector<int>& parameters) : UnaryScopedEffect(scope, image, parameters) {}

	Atom apply(ObjectIdx v1) const {
		assert(applicable(v1));
		// value(?c) := +(value(?c), 1)
		return Atom(_affected, v1 + 1);
	}

	std::string getName() const {
		return std::string("value(?c) := +(value(?c), 1)");
	}

};

class IncrementActionEffect0Implementer : public gecode::EffectTranslator {
public:

  void addConstraint( ScopedEffect::cptr eff, ComplexActionManager& manager ) const {
    auto y_var = manager.resolveY(eff->getAffected());
    auto x_var = manager.resolveX(eff->getScope()[0]);
    Gecode::IntArgs     coeffs(2);
    Gecode::IntVarArgs  vars(2);
    coeffs[0] = 1;
    coeffs[1] = -1;
    vars[0] = y_var;
    vars[1] = x_var;
    Gecode::linear( *(manager.getCSP()), coeffs, vars, Gecode::IRT_EQ, 1 );
  }

};




class DecrementActionEffect0 : public UnaryScopedEffect {
public:
	DecrementActionEffect0(const VariableIdxVector& scope, const VariableIdxVector& image, const std::vector<int>& parameters) : UnaryScopedEffect(scope, image, parameters) {}

	Atom apply(ObjectIdx v1) const {
		assert(applicable(v1));
		// value(?c) := -(value(?c), 1)
		return Atom(_affected, v1 - 1);
	}

	std::string getName() const {
		return std::string("value(?c) := -(value(?c), 1)");
	}
};

class DecrementActionEffect0Implementer : public gecode::EffectTranslator {
public:
  void addConstraint( ScopedEffect::cptr eff, ComplexActionManager& manager ) const {
	//assert(0); // Just checking
    auto y_var = manager.resolveY(eff->getAffected());
    auto x_var = manager.resolveX(eff->getScope()[0]);
    Gecode::IntArgs     coeffs(2);
    Gecode::IntVarArgs  vars(2);
    coeffs[0] = 1;
    coeffs[1] = -1;
    vars[0] = y_var;
    vars[1] = x_var;
    Gecode::linear( *(manager.getCSP()), coeffs, vars, Gecode::IRT_EQ, -1 );
  }

};


/*********************************************/
/* Method factories                          */
/*********************************************/


class ComponentFactory {
public:
    static Action::cptr instantiateAction(
        const std::string& classname,
		const ObjectIdxVector& binding,
		const ObjectIdxVector& derived,
        const std::vector<VariableIdxVector>& appRelevantVars,
        const std::vector<VariableIdxVector>& effRelevantVars,
        const std::vector<VariableIdxVector>& effAffectedVars
    ) {
        Action* pointer = nullptr;

        if (false) {}

        else if(classname == "IncrementAction") {
			ScopedConstraint::vcptr constraints = {

			};
			ScopedEffect::vcptr effects = {
				new IncrementActionEffect0(effRelevantVars[0], effAffectedVars[0], binding)
			};
			pointer = new IncrementAction(binding, constraints, effects);
        }

        else if(classname == "DecrementAction") {
			ScopedConstraint::vcptr constraints = {

			};
			ScopedEffect::vcptr effects = {
				new DecrementActionEffect0(effRelevantVars[0], effAffectedVars[0], binding)
			};
			pointer = new DecrementAction(binding, constraints, effects);
        }
        else throw std::runtime_error("Unknown action name.");

        return pointer;
    }

    static ScopedConstraint::vcptr instantiateGoal(const std::vector<VariableIdxVector>& appRelevantVars) {
    const ObjectIdxVector binding;  // The goal has empty binding
		return {
		    new LTConstraint(appRelevantVars[0], {}),
			new LTConstraint(appRelevantVars[1], {}),
			new LTConstraint(appRelevantVars[2], {})
		};
    }

    static ScopedConstraint::cptr instantiateConstraint(const std::string& classname, const ObjectIdxVector& parameters, const VariableIdxVector& variables) {
        return external->instantiateConstraint(classname, parameters, variables);
    }
};

/* Generate the whole planning problem */
void generate(const std::string& data_dir, Problem& problem);
