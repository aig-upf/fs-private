
#pragma once

#include <cassert>
#include <iosfwd>
#include <fs0_types.hxx>
#include <constraints/problem_constraints.hxx>

namespace fs0 {


/**
 * 
 */
class ConstraintManager
{
protected:
	//! An arc is a pair with a procedure and the index of the relevant variable (either 0 or 1)
	typedef std::pair<ProblemConstraint::cptr, unsigned> Arc;
 	// typedef boost::container::flat_set<Arc> ArcSet;
	typedef std::set<Arc> ArcSet;
	
	typedef std::vector<ProblemConstraint::cptr> PConstraintPtrVct;
	
	//! State constraints
	ProblemConstraint::vctr sconstraints;
	
	//! Goal constraints
	ProblemConstraint::vctr gconstraints;
	
	//! The worklists necessary for the AC3 algorithm: State constraints and goal constraints
	ArcSet SCWorklist;
	ArcSet GCWorklist;
	
	//! We keep a number of indexes of the different constraints for efficiency reasons
	PConstraintPtrVct s_unary_contraints;
	PConstraintPtrVct s_binary_contraints;
	PConstraintPtrVct s_n_ary_contraints;
	PConstraintPtrVct g_unary_contraints;
	PConstraintPtrVct g_binary_contraints;
	PConstraintPtrVct g_n_ary_contraints;
	
	VariableIdxVector s_relevant;
	VariableIdxVector g_relevant;


public:
	
	ConstraintManager(const ProblemConstraint::vctr& goalConstraints, const ProblemConstraint::vctr& stateConstraints);
	
	//! Precompute some of the structures that we'll need later on.
	void initialize();
	
	const VariableIdxVector& getStateConstraintRelevantVariables() const;
	const VariableIdxVector& getGoalConstraintRelevantVariables() const;
	
	//! Indexes pointers to the constraints in three different vectors: unary, binary and n-ary constraints.
	void indexConstraintsByArity(const ProblemConstraint::vctr& constraints,
								 PConstraintPtrVct& unary,
							     PConstraintPtrVct& binary,
								 PConstraintPtrVct& n_ary
								);
	
	//! Initializes a worklist. `constraints` is expected to have only binary constraints.
	void initializeAC3Worklist(const PConstraintPtrVct& constraints, ArcSet& worklist);
	
	Constraint::Output unaryFiltering(const DomainMap& domains, const PConstraintPtrVct& constraints) const;
	
	Constraint::Output filter(const DomainMap& domains, 
							const PConstraintPtrVct& unary, 
							const PConstraintPtrVct& binary,
							const PConstraintPtrVct& n_ary,
							const ArcSet& AC3Worklist
	) const;

	Constraint::Output filterWithStateConstraints(const DomainMap& domains) const;
	
	//! Prunes the domains of the given state
	Constraint::Output filterWithGoalConstraints(const DomainMap& domains) const;
	
	//!
	void loadConstraintDomains(const DomainMap& domains, const PConstraintPtrVct& constraints) const;
	void emptyConstraintDomains(const PConstraintPtrVct& constraints) const;
	
	Constraint::Output filter_global_constraints(const PConstraintPtrVct& constraints) const;

	//! AC3 filtering
	Constraint::Output filter_binary_constraints(const PConstraintPtrVct& constraints, ArcSet& worklist) const;
	
/*
	void printArcSet(const ArcSet& arcs) const {
		const auto problemInfo = Problem::getCurrentProblem()->getProblemInfo();
		std::cout << "arc-set: " << std::endl;
		for (const auto& arc:arcs) {
			std::cout << "["  << arc.first << ", " << problemInfo->getVariableName(arc.second) << "]" << std::endl;
		}
	}
*/

	//! Returns true iff the given non-relaxed state satisfies all of the goal constraints.
	bool checkGoalConstraintsSatisfied(const State& s) const;
	
	//! Return true iff all variable domain are non-empty
	bool checkConsistency(const DomainMap& domains) const;
	
protected:
	
	Arc select(ArcSet& worklist) const;

	//
	VariableIdxVector indexRelevantVariables(ProblemConstraint::vctr constraints);
};



} // namespaces

