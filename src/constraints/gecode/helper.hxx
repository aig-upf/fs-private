

#pragma once

#include <fs0_types.hxx>
#include <problem_info.hxx>
#include <problem.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <constraints/gecode/translator_repository.hxx>

#include <gecode/int.hh>


using namespace Gecode;

namespace fs0 { namespace gecode {

/**
 *  Some helper methods related to the construction of Gecode CSPs.
 */
class Helper {
public:
	//!
	static unsigned processVariable(Gecode::Space& csp, VariableIdx var, Gecode::IntVarArgs& varArray) {
		const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
		ProblemInfo::ObjectType varType = info.getVariableGenericType( var );
		if ( varType == ProblemInfo::ObjectType::INT ) {
			auto bounds = info.getVariableBounds(var);
			Gecode::IntVar x( csp, bounds.first, bounds.second );
			varArray << x;
		}
		else if ( varType == ProblemInfo::ObjectType::BOOL ) {
			varArray << Gecode::IntVar( csp, 0, 1 );
		}
		else if ( varType == ProblemInfo::ObjectType::OBJECT ) {
			const ObjectIdxVector& values = info.getVariableObjects( var );
			std::vector<int> _values( values.size() );
			for ( unsigned j = 0; j < values.size(); j++ ) {
				_values[j] = values[j];
			}
			varArray << Gecode::IntVar( csp, Gecode::IntSet( _values.data(), values.size() ));
		}
		return varArray.size() - 1;
	}

	static void addEqualityConstraint(SimpleCSP& csp, const gecode::GecodeCSPTranslator& translator, VariableIdx variable, bool value) {
		addEqualityConstraint(csp, translator, variable, (value ? 1 : 0));
	}

	// MRJ: This overload will be necessary as soon as int and ObjectIdx cease to be the same thing
	//void addEqualityConstraint(SimpleCSP& csp,  VariableIdx variable, int  value );

	//! Adds constraint of the form $variable = value$ to the CSP
	static void addEqualityConstraint(SimpleCSP& csp, const gecode::GecodeCSPTranslator& translator, VariableIdx variable, ObjectIdx value) {
		auto csp_var = translator.resolveVariable(csp, variable, GecodeCSPTranslator::VariableType::Input);
		rel( csp, csp_var, IRT_EQ, value ); // v = value
	}


	//! Adds constraint of the form $variable \in values$ to the CSP
	static void addMembershipConstraint(SimpleCSP& csp, const gecode::GecodeCSPTranslator& translator, VariableIdx variable, DomainPtr values) {
		auto csp_var = translator.resolveVariable(csp, variable, GecodeCSPTranslator::VariableType::Input);

		// MRJ: variable \in dom
		TupleSet valueSet;
		for ( auto v : *values ) {
			valueSet.add( IntArgs( 1, v ));
		}
		valueSet.finalize();
		extensional( csp, IntVarArgs() << csp_var, valueSet ); // MRJ: v \in valueSet
	}

	//! Adds constraint of the form $lb <= variable <= ub$ to the CSP
	static void addBoundsConstraint(SimpleCSP& csp, const gecode::GecodeCSPTranslator& translator, VariableIdx variable, int lb, int ub) {
		auto csp_var = translator.resolveVariable(csp, variable, GecodeCSPTranslator::VariableType::Input);
		dom( csp, csp_var, lb, ub); // MRJ: lb <= variable <= ub
	}

	//! Adds constraint of the form $min <= variable <= max$ to the CSP,
	//! where min and max are the minimum and maximum values defined for
	//! the type of variable.
	static void addBoundsConstraintFromDomain(SimpleCSP& csp, const gecode::GecodeCSPTranslator& translator, VariableIdx variable) {
		auto csp_var = translator.resolveVariable(csp, variable, GecodeCSPTranslator::VariableType::Input);
		const auto& info = Problem::getCurrentProblem()->getProblemInfo();
		TypeIdx type = info.getVariableType(variable);
		if (!info.hasVariableBoundedDomain(type)) return; // Nothing to do in this case
		const auto& bounds = info.getVariableBounds(type);
		dom( csp, csp_var, bounds.first, bounds.second); // MRJ: bounds.first <= variable <= bounds.second
	}


	static void translateConstraints(gecode::SimpleCSP& csp, const gecode::GecodeCSPTranslator& translator, const ScopedConstraint::vcptr& constraints) {
		for (ScopedConstraint::cptr constraint:constraints) {
			auto transObj = TranslatorRepository::instance().getConstraintTranslator( typeid(*constraint) );
			assert(transObj != nullptr);
			transObj->addConstraint(csp, translator, constraint);
		}
	}

	static void translateEffects(gecode::SimpleCSP& csp, const gecode::GecodeCSPTranslator& translator, const ScopedEffect::vcptr& effects) {
		for (ScopedEffect::cptr effect:effects) {
			auto transObj = TranslatorRepository::instance().getEffectTranslator( typeid(*effect) );
			assert(transObj != nullptr);
			transObj->addConstraint(csp, translator, effect);
		}
	}

	//! Adds the RPG-layer-dependent constraints to the CSP.
	static void addRelevantVariableConstraints(SimpleCSP& csp, const gecode::GecodeCSPTranslator& translator, const VariableIdxVector& scope, const RelaxedState& layer) {
		// Loop over the domains of each of the relevant variables in the RPG layer and process them one by one.
		for (VariableIdx variable:scope) {
			const DomainPtr& domain = layer.getValues(variable);
			if ( domain->size() == 1 ) {
				addEqualityConstraint( csp, translator, variable, *(domain->begin()) );
			} else {
				ObjectIdx lb = *(domain->begin());
				ObjectIdx ub = *(domain->rbegin());
				
				if ( domain->size() == (ub - lb) ) { // MRJ: Check this is a safe assumption
					addBoundsConstraint( csp, translator, variable, lb, ub );
				} else { // MRJ: worst case (performance wise) yet I think it can be optimised in a number of ways
					addMembershipConstraint( csp, translator, variable, domain );
				}
			}
		}
	}
	
	static void postBranchingStrategy(SimpleCSP& csp) {
		// TODO posting a particular branching strategy might make sense to prioritize some branching strategy?
		branch(csp, csp._X, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
	}
};

} } // namespaces