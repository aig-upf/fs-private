
#pragma once

#include <cassert>
#include <iosfwd>
#include <fs0_types.hxx>
#include <constraints/scoped_constraint.hxx>

namespace fs0 {


/**
 * 
 */
class ConstraintManager
{
protected:
	//! An arc is a pair with a procedure and the index of the relevant variable (either 0 or 1)
	typedef std::pair<ScopedConstraint::cptr, unsigned> Arc;
 	typedef boost::container::flat_set<Arc> ArcSet;
// 	typedef std::set<Arc> ArcSet;
	
	
	//! The set of constraints that we manage
	//! This will be a reference to either the vector of constraints stored in the Problem object
	//! or to the vector of constraints stored in some action.
	const ScopedConstraint::vcptr& _constraints;
	
	//! The worklists necessary for the AC3 algorithm: State constraints and goal constraints
	ArcSet AC3Worklist;
	
	//! We keep a number of indexes of the different constraints for efficiency reasons
	ScopedConstraint::vcptr unary_constraints;
	ScopedConstraint::vcptr binary_constraints;
	ScopedConstraint::vcptr n_ary_constraints;
	
	VariableIdxVector relevant;

public:
	//! Constructs a manager handling the given set of constraints
	ConstraintManager(const ScopedConstraint::vcptr& constraints);
	
	//! Precompute some of the structures that we'll need later on.
	void initialize();
	
	const VariableIdxVector& getAllRelevantVariables() const;
	
	//! Indexes pointers to the constraints in three different vectors: unary, binary and n-ary constraints.
	void indexConstraintsByArity();
	
	//! Initializes a worklist. `constraints` is expected to have only binary constraints.
	void initializeAC3Worklist(const ScopedConstraint::vcptr& constraints, ArcSet& worklist);
	
	//! Filter the domains with all the constraints
	ScopedConstraint::Output filter(const DomainMap& domains) const;
	
	//!
	void loadConstraintDomains(const DomainMap& domains, const ScopedConstraint::vcptr& constraints) const;
	void emptyConstraintDomains(const ScopedConstraint::vcptr& constraints) const;
	
	//! Returns true iff the given non-relaxed state satisfies all the constraints.
	bool checkSatisfaction(const State& s) const;
	
	//! Return true iff all variable domain are non-empty
	static bool checkConsistency(const DomainMap& domains);
	
protected:
	
	//! The AC-3 selection algorithm
	Arc select(ArcSet& worklist) const;

	//! Helper to index all the variables that are relevant to any of the given constraints
	static VariableIdxVector indexRelevantVariables(const ScopedConstraint::vcptr& constraints);
	
	//! Simply filter out the domains that do not satisfy each of the unary constraints
	ScopedConstraint::Output unaryFiltering(const DomainMap& domains) const;
	
	//! AC3 filtering
	ScopedConstraint::Output binaryFiltering(ArcSet& worklist) const;
	
	//! Apply whatever custom filtering algorithm the constraint has
	ScopedConstraint::Output globalFiltering() const;	
};



} // namespaces

