

#pragma once

#include <fs0_types.hxx>
#include <problem_info.hxx>
#include <constraints/direct/compiled.hxx>
#include <languages/fstrips/language.hxx>
#include <gecode/int.hh>

using namespace Gecode;

namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

class SimpleCSP; class GecodeCSPVariableTranslator; enum class CSPVariableType;

/**
 *  Some helper methods related to the construction of Gecode CSPs.
 */
class Helper {
public:
	//! Creates the actual Gecode CSP variable and assigns it a proper domain based on the planning problem variable type
	static Gecode::IntVar processPlanningVariable(Gecode::Space& csp, VariableIdx var);
	
	static Gecode::IntVar processTemporaryVariable(Gecode::Space& csp, VariableIdx var, TypeIdx typeId);
	
	static Gecode::IntVar processVariable(Gecode::Space& csp, VariableIdx var, ProblemInfo::ObjectType type);
	
	//!
	static void registerPlanningVariable(Gecode::Space& csp, VariableIdx planning_var, CSPVariableType type, Gecode::IntVarArgs& variables, GecodeCSPVariableTranslator& translator);
	
	//!
	static void registerTemporaryVariable(Gecode::Space& csp, VariableIdx planning_var, Gecode::IntVarArgs& variables, GecodeCSPVariableTranslator& translator, TypeIdx typeId);
	
	static void registerTemporaryOutputVariable(Gecode::Space& csp, VariableIdx planning_var, Gecode::IntVarArgs& variables, GecodeCSPVariableTranslator& translator, TypeIdx typeId);
	
	//!
	static void addEqualityConstraint(SimpleCSP& csp, const GecodeCSPVariableTranslator& translator, VariableIdx variable, bool value);

	// MRJ: This overload will be necessary as soon as int and ObjectIdx cease to be the same thing
	//void addEqualityConstraint(SimpleCSP& csp,  VariableIdx variable, int  value );

	//! Adds constraint of the form $variable = value$ to the CSP
	static void addEqualityConstraint(SimpleCSP& csp, const GecodeCSPVariableTranslator& translator, VariableIdx variable, ObjectIdx value);

	//! Adds constraint of the form $variable \in values$ to the CSP
	static void addMembershipConstraint(SimpleCSP& csp, const GecodeCSPVariableTranslator& translator, VariableIdx variable, DomainPtr values);

	//! Adds constraint of the form $lb <= variable <= ub$ to the CSP
	static void addBoundsConstraint(SimpleCSP& csp, const GecodeCSPVariableTranslator& translator, VariableIdx variable, int lb, int ub);

	//! Adds constraint of the form $min <= variable <= max$ to the CSP,
	//! where min and max are the minimum and maximum values defined for
	//! the type of variable.
	static void addBoundsConstraintFromDomain(SimpleCSP& csp, const GecodeCSPVariableTranslator& translator, VariableIdx variable);

	/*
	//! Returns the translator that corresponds to a given constraint, if available, or throws a fatal exception otherwise.
	static ConstraintTranslator::ptr getConstraintTranslator(const ScopedConstraint& constraint) {
		const std::type_info& type = typeid(constraint);
		ConstraintTranslator::ptr componentTranslator = TranslatorRepository::instance().getConstraintTranslator(type);
		
		if (componentTranslator) return componentTranslator;
			
		// If no translator was registered for the concrete component, we try out with the few (extensional) generic translators that we have.
		componentTranslator = TranslatorRepository::instance().getConstraintTranslator(constraint.getDefaultTypeId());
		
		// Otherwise, we cannot continue
		if (!componentTranslator) throw std::runtime_error("No ConstraintTranslator registered for type " + boost::units::detail::demangle(type.name()));
		return componentTranslator;
	}
	
	//! Returns the translator that corresponds to a given effect, if available, or throws a fatal exception otherwise.
	static EffectTranslator::ptr getEffectTranslator(const ScopedEffect& effect) {
		const std::type_info& type = typeid(effect);
		auto componentTranslator = TranslatorRepository::instance().getEffectTranslator(type);
		
		if (componentTranslator) return componentTranslator;
		
		// If no translator was registered for the concrete component, we try out with the few (extensional) generic translators that we have.
		componentTranslator = TranslatorRepository::instance().getEffectTranslator(effect.getDefaultTypeId());
		
		// Otherwise, we cannot continue
		if (!componentTranslator) throw std::runtime_error("No EffectTranslator registered for type " + boost::units::detail::demangle(type.name()));
		return componentTranslator;
	}

	static void translateConstraints(SimpleCSP& csp, const GecodeCSPVariableTranslator& translator, const ScopedConstraint::vcptr& constraints) {
		for (ScopedConstraint::cptr constraint:constraints) {
			getConstraintTranslator(*constraint)->registerConstraints(csp, translator, constraint);
		}
	}

	static void translateEffects(SimpleCSP& csp, const GecodeCSPVariableTranslator& translator, const ScopedEffect::vcptr& effects) {
		for (ScopedEffect::cptr effect:effects) {
			getEffectTranslator(*effect)->registerConstraints(csp, translator, effect);
		}
	}
	*/

	//! Adds the RPG-layer-dependent constraints to the CSP.
	static void addRelevantVariableConstraints(SimpleCSP& csp, const GecodeCSPVariableTranslator& translator, const VariableIdxVector& scope, const RelaxedState& layer);
	
	static void postBranchingStrategy(SimpleCSP& csp);
	
	static Gecode::TupleSet extensionalize(const fs::Term::cptr term, const VariableIdxVector& scope);

};

} } // namespaces