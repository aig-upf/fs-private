
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
#include <constraints/gecode/translators.hxx>
#include <constraints/gecode/csp_translator.hxx>

#include <constraints/gecode/helper.hxx>
#include <component_factory.hxx>


using namespace fs0;
using namespace fs0::gecode;

extern std::unique_ptr<External> external;

/*********************************************/
/* The actions                               */
/*********************************************/

class MoveAction : public Action {

protected:
    static const std::string name_;
    static const ActionSignature signature_;

public:
	MoveAction(const ObjectIdxVector& binding, const ScopedConstraint::vcptr& constraints, const ScopedEffect::vcptr& effects) : Action(binding, constraints, effects) {}

    const std::string& getName() const { return name_; }
    const ActionSignature& getSignature() const { return signature_; }
};

/*********************************************/
/* Constraint and Effect components          */
/*********************************************/
class MoveActionConstraint0 : public UnaryParametrizedScopedConstraint {
public:
	MoveActionConstraint0(const VariableIdxVector& scope, const std::vector<int>& parameters) : UnaryParametrizedScopedConstraint(scope, parameters) {}

    bool isSatisfied(ObjectIdx v1) const {
		// not =(next(?d(), blank()), nowhere())
		// not =(next(?d(), blank()), nowhere())
		return (bool) !(external->get_next(_binding[0], v1) == 3);
	}
	
	virtual std::ostream& print(std::ostream& os) const {
		os << "not =(next(?d(), blank()), nowhere())";
		return os;
	}
};


class MoveActionEffect1 : public UnaryScopedEffect {
public:
	MoveActionEffect1(const VariableIdxVector& scope, VariableIdx affected, const std::vector<int>& parameters) : UnaryScopedEffect(scope, affected, parameters) {}

	Atom apply(ObjectIdx v1) const { 
		assert(applicable(v1));
		// blank() := next(?d, blank())
		return Atom(_affected, external->get_next(_binding[0], v1));
	}

	virtual std::ostream& print(std::ostream& os) const {
		os << "blank() := next(?d, blank())";
		return os;
	}
};

// (assign (tile_at (blank)) (tile_at (next ?d (blank))))
class MoveActionEffect2 : public ScopedEffect {
public:
	MoveActionEffect2(const VariableIdxVector& scope, VariableIdx affected, const std::vector<int>& parameters) : ScopedEffect(scope, affected, parameters) {}

	Atom apply(const ObjectIdxVector& values) const { throw std::runtime_error("Nested effect that can only be handled with gecode"); }

	virtual std::ostream& print(std::ostream& os) const {
		os << "tile_at (blank) := tile_at (next ?d (blank))";
		return os;
	}
};

class MoveActionEffect3 : public ScopedEffect {
public:
	MoveActionEffect3(const VariableIdxVector& scope, VariableIdx affected, const std::vector<int>& parameters) : ScopedEffect(scope, affected, parameters) {}

	Atom apply(const ObjectIdxVector& values) const { throw std::runtime_error("Nested effect that can only be handled with gecode"); }

	virtual std::ostream& print(std::ostream& os) const {
		os << "tile_at(next ?d (blank)) := none";
		return os;
	}
};

// (assign (tile_at (blank)) (tile_at (next ?d (blank))))
class MoveActionEffect2Translator : public EffectTranslator  {
public:

	void registerVariables(SimpleCSP& csp, GecodeCSPTranslator& translator, ScopedEffect::cptr effect, Gecode::IntVarArgs& variables) const {
		const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
		
		unsigned tile_at_id = info.getFunctionId("tile_at"); // TODO - This should be cached somewhere, e.g. as an object attribute.
		unsigned next_id = info.getFunctionId("next");

		TypeIdx tile_at_t = info.getFunctionData(tile_at_id).getCodomainType();
		TypeIdx next_t = info.getFunctionData(next_id).getCodomainType();
		
		
		// Register the necessary csp-temporary variables
		Helper::registerTemporaryVariable(csp, 1, variables, translator, tile_at_t); // tmp-X
		Helper::registerTemporaryVariable(csp, 2, variables, translator, next_t);  // tmp-Z
		
	}

	void registerConstraints(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedEffect::cptr effect) const {
		const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
		
		auto tile_at_data = info.getFunctionData(info.getFunctionId("tile_at")); // TODO - This should be cached somewhere, e.g. as an object attribute.
		unsigned blank_id = info.getVariableId("blank");

		
		// Retrieve necessary CSP variables:
		const Gecode::IntVar& blank   = translator.resolveVariable(csp, blank_id, CSPVariableType::Input);
		Gecode::IntVarArgs tile_at_in = translator.resolveFunction(csp, tile_at_data, CSPVariableType::Input);
		Gecode::IntVarArgs tile_at_out = translator.resolveFunction(csp, tile_at_data, CSPVariableType::Output);
		const Gecode::IntVar& tmpX    = translator.resolveVariable(csp, 1, CSPVariableType::Temporary); // The integer indexes are key!
		const Gecode::IntVar& tmpZ    = translator.resolveVariable(csp, 2, CSPVariableType::Temporary);
		
		// REGISTER THE EXTENSIONAL CONSTRAINT FOR Z = next(d, blank)
		const ObjectIdx d_value = effect->getBinding()[0];
		const ObjectIdxVector& blank_values = info.getVariableObjects(blank_id);
		
		// Tupleset of the form {<z,x> | z = next(d, z)}
		Gecode::TupleSet tuples;
		for(ObjectIdx blank_value:blank_values) {
			try {
				auto res = external->get_next(d_value, blank_value);
				tuples.add(Gecode::IntArgs(2, res, blank_value));
			} catch(const std::out_of_range& e) {} // If the effect produces an exception, we simply consider it non-applicable and go on.
		}
		tuples.finalize();
		Gecode::extensional(csp, IntVarArgs() << tmpZ << blank, tuples); // Order matters - variable order must be the same than in the tupleset
		
		// left-hand side
		Gecode::element(csp, tile_at_out, blank, tmpX);
		
		// right-hand side
		Gecode::element(csp, tile_at_in, tmpZ, tmpX);
	}
};



class MoveActionEffect3Translator : public EffectTranslator  {
public:

	void registerVariables(SimpleCSP& csp, GecodeCSPTranslator& translator, ScopedEffect::cptr effect, Gecode::IntVarArgs& variables) const {
		const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
		unsigned next_id = info.getFunctionId("next");
		TypeIdx next_t = info.getFunctionData(next_id).getCodomainType();
		
		// Register the necessary csp-temporary variables
		// NOTE: THIS WILL ALREADY BE REGISTERED BY THE PREVIOUS EFFECT... NO PROBLEM, INDEED WE NEED TO ACHIEVE THIS, AS WE WANT TO RECUSE THE 
		// TEMPORARY VARIABLE THAT IS DERIVED FROM THE SAME SUBEXPRESSION.
		Helper::registerTemporaryVariable(csp, 2, variables, translator, next_t);  // tmp-Z
		
	}

	void registerConstraints(SimpleCSP& csp, const GecodeCSPTranslator& translator, ScopedEffect::cptr effect) const {
		const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
		
		auto tile_at_data = info.getFunctionData(info.getFunctionId("tile_at")); // TODO - This should be cached somewhere, e.g. as an object attribute.
		
		// Retrieve necessary CSP variables:
		Gecode::IntVarArgs tile_at_out = translator.resolveFunction(csp, tile_at_data, CSPVariableType::Output);
		const Gecode::IntVar& tmpZ    = translator.resolveVariable(csp, 2, CSPVariableType::Temporary);
		
		// THE EXTENSIONAL CONSTRAINT FOR Z = next(d, blank) IS ALREADY REGISTERED BY THE PREVIOUS EFFECT
		
		ObjectIdx none_value = info.getObjectId("none");
		
		Gecode::element(csp, tile_at_out, tmpZ, none_value);
	}
};






/*********************************************/
/* Method factories                          */
/*********************************************/


class ComponentFactory : public BaseComponentFactory {
public:
    Action::cptr instantiateAction(
        const std::string& classname,
		const ObjectIdxVector& binding,
		const ObjectIdxVector& derived,
        const std::vector<VariableIdxVector>& appRelevantVars,
        const std::vector<VariableIdxVector>& effRelevantVars,
        const VariableIdxVector& effAffectedVars
    ) const {
        Action* pointer = nullptr;

        const ObjectIdxVector& _binding = binding; // syntactic sugar

        if (false) {}
        
        else if(classname == "MoveAction") {
			ScopedConstraint::vcptr constraints = {
				new MoveActionConstraint0(appRelevantVars[0], binding), // not =(next(?d(), blank()), nowhere())
			};
			ScopedEffect::vcptr effects = {
				new MoveActionEffect1(effRelevantVars[1], effAffectedVars[1], binding), // blank() := next(?d, blank())
				new MoveActionEffect2({}, {}, binding), // (assign (tile_at (blank)) (tile_at (next ?d (blank))))
				new MoveActionEffect3({}, {}, binding), //  tile_at(next ?d (blank)) := none
			};
			pointer = new MoveAction(binding, constraints, effects);
        }
        else throw std::runtime_error("Unknown action name.");

        return pointer;
    }

    ScopedConstraint::vcptr instantiateGoal(const std::vector<VariableIdxVector>& appRelevantVars) const {
    const ObjectIdxVector binding;  // The goal has empty binding
		return {
		    new EQXConstraint(appRelevantVars[0], {25}), // =(tile_at(tl), t8())
			new EQXConstraint(appRelevantVars[1], {24}), // =(tile_at(tm), t7())
			new EQXConstraint(appRelevantVars[2], {23}), // =(tile_at(tr), t6())
			new EQXConstraint(appRelevantVars[3], {21}), // =(tile_at(mm), t4())
			new EQXConstraint(appRelevantVars[4], {18}), // =(tile_at(mr), t1())
			new EQXConstraint(appRelevantVars[5], {19}), // =(tile_at(bl), t2())
			new EQXConstraint(appRelevantVars[6], {22}), // =(tile_at(bm), t5())
			new EQXConstraint(appRelevantVars[7], {20}), // =(tile_at(br), t3())
		};
    }

    ScopedConstraint::cptr instantiateConstraint(const std::string& classname, const ObjectIdxVector& parameters, const VariableIdxVector& variables) const {
        return external->instantiateConstraint(classname, parameters, variables);
    }
    
    
    std::map<std::string, Function> instantiateFunctions() const {
		return {
			{"next", [](const ObjectIdxVector& parameters){ return external->get_next(parameters) ;}}
		};
	}
};

/* Generate the whole planning problem */
void generate(const rapidjson::Document& data, const std::string& data_dir, Problem& problem);
