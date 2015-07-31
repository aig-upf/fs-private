
#include <constraints/gecode/helper.hxx>
#include <problem_info.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <problem.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <relaxed_state.hxx>
#include <boost/units/detail/utility.hpp>


namespace fs0 { namespace gecode {
	
	
Gecode::IntVar Helper::processPlanningVariable(Gecode::Space& csp, VariableIdx var) {
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	return processVariable(csp, var, info.getVariableGenericType(var));
}

Gecode::IntVar Helper::processTemporaryVariable(Gecode::Space& csp, VariableIdx var, TypeIdx typeId) {
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	return processVariable(csp, var, info.getGenericType(typeId));
}

Gecode::IntVar Helper::processVariable(Gecode::Space& csp, VariableIdx var, ProblemInfo::ObjectType type) {
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	if ( type == ProblemInfo::ObjectType::INT ) {
		auto bounds = info.getVariableBounds(var);
		return Gecode::IntVar( csp, bounds.first, bounds.second );
	}
	else if ( type == ProblemInfo::ObjectType::BOOL ) {
		return Gecode::IntVar( csp, 0, 1 );
	}
	else {
		assert(type == ProblemInfo::ObjectType::OBJECT);
		const ObjectIdxVector& values = info.getVariableObjects( var );
		std::vector<int> _values( values.size() );
		for ( unsigned j = 0; j < values.size(); j++ ) {
			_values[j] = values[j];
		}
		return Gecode::IntVar( csp, Gecode::IntSet( _values.data(), values.size() ));
	}
}

//!
void Helper::registerPlanningVariable(Gecode::Space& csp, VariableIdx planning_var, CSPVariableType type, Gecode::IntVarArgs& variables, GecodeCSPTranslator& translator) {
	auto csp_var = processPlanningVariable(csp, planning_var);
	if (translator.registerCSPVariable(planning_var, type, variables.size())) {
		variables << csp_var;
	}
}

//!
void Helper::registerTemporaryVariable(Gecode::Space& csp, VariableIdx planning_var, Gecode::IntVarArgs& variables, GecodeCSPTranslator& translator, TypeIdx typeId) {
	auto csp_var = processTemporaryVariable(csp, planning_var, typeId);
	if (translator.registerCSPVariable(planning_var, CSPVariableType::Temporary, variables.size())) {
		variables << csp_var;
	}
}

void Helper::registerTemporaryOutputVariable(Gecode::Space& csp, VariableIdx planning_var, Gecode::IntVarArgs& variables, GecodeCSPTranslator& translator, TypeIdx typeId) {
	auto csp_var = processTemporaryVariable(csp, planning_var, typeId);
	if (translator.registerCSPVariable(planning_var, CSPVariableType::TemporaryOutput, variables.size())) {
		variables << csp_var;
	}
}

//!
void Helper::addEqualityConstraint(SimpleCSP& csp, const gecode::GecodeCSPTranslator& translator, VariableIdx variable, bool value) {
	addEqualityConstraint(csp, translator, variable, (value ? 1 : 0));
}

//! Adds constraint of the form $variable = value$ to the CSP
void Helper::addEqualityConstraint(SimpleCSP& csp, const gecode::GecodeCSPTranslator& translator, VariableIdx variable, ObjectIdx value) {
	auto csp_var = translator.resolveVariable(csp, variable, CSPVariableType::Input);
	rel( csp, csp_var, IRT_EQ, value ); // v = value
}


//! Adds constraint of the form $variable \in values$ to the CSP
void Helper::addMembershipConstraint(SimpleCSP& csp, const gecode::GecodeCSPTranslator& translator, VariableIdx variable, DomainPtr values) {
	auto csp_var = translator.resolveVariable(csp, variable, CSPVariableType::Input);

	// MRJ: variable \in dom
	TupleSet valueSet;
	for ( auto v : *values ) {
		valueSet.add( IntArgs( 1, v ));
	}
	valueSet.finalize();
	extensional( csp, IntVarArgs() << csp_var, valueSet ); // MRJ: v \in valueSet
}

//! Adds constraint of the form $lb <= variable <= ub$ to the CSP
void Helper::addBoundsConstraint(SimpleCSP& csp, const gecode::GecodeCSPTranslator& translator, VariableIdx variable, int lb, int ub) {
	auto csp_var = translator.resolveVariable(csp, variable, CSPVariableType::Input);
	dom( csp, csp_var, lb, ub); // MRJ: lb <= variable <= ub
}

//! Adds constraint of the form $min <= variable <= max$ to the CSP,
//! where min and max are the minimum and maximum values defined for
//! the type of variable.
void Helper::addBoundsConstraintFromDomain(SimpleCSP& csp, const gecode::GecodeCSPTranslator& translator, VariableIdx variable) {
	auto csp_var = translator.resolveVariable(csp, variable, CSPVariableType::Input);
	const auto& info = Problem::getCurrentProblem()->getProblemInfo();
	TypeIdx type = info.getVariableType(variable);
	if (!info.hasVariableBoundedDomain(type)) return; // Nothing to do in this case
	const auto& bounds = info.getVariableBounds(type);
	dom( csp, csp_var, bounds.first, bounds.second); // MRJ: bounds.first <= variable <= bounds.second
}

//! Adds the RPG-layer-dependent constraints to the CSP.
void Helper::addRelevantVariableConstraints(SimpleCSP& csp, const gecode::GecodeCSPTranslator& translator, const VariableIdxVector& scope, const RelaxedState& layer) {
	// Loop over the domains of each of the relevant variables in the RPG layer and process them one by one.
	for (VariableIdx variable:scope) {
		const DomainPtr& domain = layer.getValues(variable);
		if ( domain->size() == 1 ) {
			addEqualityConstraint( csp, translator, variable, *(domain->begin()) );
		} else {
			ObjectIdx lb = *(domain->begin());
			ObjectIdx ub = *(domain->rbegin());
			
			if ( domain->size() == static_cast<unsigned>(ub - lb) ) { // MRJ: Check this is a safe assumption - We can guarantee it is unsigned, since the elements in the domain are ordered
				addBoundsConstraint( csp, translator, variable, lb, ub );
			} else { // MRJ: worst case (performance wise) yet I think it can be optimised in a number of ways
				addMembershipConstraint( csp, translator, variable, domain );
			}
		}
	}
}


Gecode::TupleSet Helper::extensionalize(const Term::cptr term, const VariableIdxVector& scope) {
	if (scope.size() > 2) throw std::runtime_error("Error trying to extensionalize a term with too high a scope");
	
	// This is not entirely correct - we need to compile static fluents differently
	// We need to examine subterm by subterm, fix those that are fixed (constants), and use whole domains for those that are either 
	// state variables or temporary variables.
	assert(0); 
	
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	Gecode::TupleSet tuples;
	
	if (scope.size() == 1) {
		
		for(ObjectIdx value:info.getVariableObjects(scope[0])) {
			ObjectIdx out = term->interpret(Projections::zip(scope, {value}));
			tuples.add(Gecode::IntArgs(2, value, out));
		}
		
	} else { // scope.size() == 2
		
		for(ObjectIdx x:info.getVariableObjects(scope[0])) {
			for(ObjectIdx y:info.getVariableObjects(scope[1])) {
				ObjectIdx out = term->interpret(Projections::zip(scope, {x, y}));
				tuples.add(Gecode::IntArgs(3, x, y, out));
			}
		}
	}
	
	tuples.finalize();
	return tuples;
}

void Helper::postBranchingStrategy(SimpleCSP& csp) {
	branch(csp, csp._X, INT_VAR_SIZE_MIN(), INT_VAL_MIN()); // TODO posting a particular branching strategy might make sense to prioritize some branching strategy?
}
	
} } // namespaces
