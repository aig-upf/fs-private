
#pragma once

#include <fs/core/fs_types.hxx>
#include <fs/core/constraints/filtering.hxx>

namespace fs0 {

class DirectConstraint;

//! A LiteCSPHandler manages CSP built with DirectConstraints
class LiteCSPHandler {
protected:
	//! An arc is a pair with a procedure and the index of the relevant variable (either 0 or 1)
	typedef std::pair<const DirectConstraint*, unsigned> Arc;
 	typedef boost::container::flat_set<Arc> ArcSet;
	
	
	//! The set of constraints that we manage.
	//! This will be a reference to either the vector of constraints stored in the Problem object
	//! or to the vector of constraints stored in some action.
	const std::vector<DirectConstraint*>& _constraints;
	
	//! The worklists necessary for the AC3 algorithm: State constraints and goal constraints
	ArcSet AC3Worklist;
	
	//! We keep a number of indexes of the different constraints for efficiency reasons
	std::vector<DirectConstraint*> unary_constraints;
	std::vector<DirectConstraint*> binary_constraints;
	std::vector<DirectConstraint*> n_ary_constraints;
	
	VariableIdxVector _relevant;

public:
	//! Constructs a manager handling the given set of constraints
	LiteCSPHandler(const std::vector<DirectConstraint*>& constraints);
	~LiteCSPHandler() = default;
	
	//! Precompute some of the structures that we'll need later on.
	void initialize();
	
	const VariableIdxVector& getAllRelevantVariables() const { return _relevant; }
	
	//! Indexes pointers to the constraints in three different vectors: unary, binary and n-ary constraints.
	void indexConstraintsByArity();
	
	//! Initializes a worklist. `constraints` is expected to have only binary constraints.
	void initializeAC3Worklist(const std::vector<DirectConstraint*>& constraints, ArcSet& worklist);
	
	//! Filter the domains with all the constraints
	FilteringOutput filter(const DomainMap& domains) const;
	
	//!
	void loadConstraintDomains(const DomainMap& domains, const std::vector<DirectConstraint*>& constraints) const;
	void emptyConstraintDomains(const std::vector<DirectConstraint*>& constraints) const;
	
	//! Return true iff all variable domain are non-empty
	static bool checkConsistency(const DomainMap& domains);

	//! Static (node-consistent) filtering of the given unary constraints
	static FilteringOutput unaryFiltering(const std::vector<DirectConstraint*>& constraints, const DomainMap& domains);
	
protected:
	
	//! The AC-3 selection algorithm
	Arc select(ArcSet& worklist) const;

	//! Helper to index all the variables that are relevant to any of the given constraints
	static VariableIdxVector indexRelevantVariables(const std::vector<DirectConstraint*>& constraints);
	
	//! Simply filter out the domains that do not satisfy each of the unary constraints
	FilteringOutput unaryFiltering(const DomainMap& domains) const;
	
	//! AC3 filtering
	FilteringOutput binaryFiltering(ArcSet& worklist) const;
	
	//! Apply whatever custom filtering algorithm the constraint has
	FilteringOutput globalFiltering() const;	
};

} // namespaces

