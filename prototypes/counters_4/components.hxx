
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
#include <constraints/gecode/translators.hxx>
#include <constraints/gecode/csp_translator.hxx>
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
				new AdditiveUnaryEffect(effRelevantVars[0], effAffectedVars[0][0], {1})
			};
			pointer = new IncrementAction(binding, constraints, effects);
        }

        else if(classname == "DecrementAction") {
			ScopedConstraint::vcptr constraints = {

			};
			ScopedEffect::vcptr effects = {
				new AdditiveUnaryEffect(effRelevantVars[0], effAffectedVars[0][0], {-1})
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
void generate(const rapidjson::Document& data, const std::string& data_dir, Problem& problem);
