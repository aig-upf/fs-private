
#pragma once

#include <vector>
#include <ostream>

#include <fs0_types.hxx>
#include <state.hxx>


namespace fs0 {

class Action;

class Projections {
public:
	//! Projects a non-relaxed state into a subset of relevant values, which returns.
	static ObjectIdxVector project(const State& s, const VariableIdxVector& scope);

	//! Helper function to copy-project a relaxed state to all of the variables relevant to a given applicable entity.
	static DomainMap projectToActionVariables(RelaxedState& state, const Action& action);
	
	/**
	 * Returns the projection of the domains of a relaxed state into a subset of variables.
	 * It is assumed that scope contains no repeated indexes.
	 */
	static DomainMap project(RelaxedState& state, const VariableIdxVector& scope);
	
	/**
	 * Returns the projection of the domains of a relaxed state into a subset of variables, cloning the projected domains.
	 * It is assumed that scope contains no repeated indexes.
	 */
	static DomainMap projectCopy(const RelaxedState& state, const VariableIdxVector& scope);

	/**
	 * Returns the projection of the domains contained in a domain map into a subset of variables.
	 * It is assumed that all the variables in scope are contained in the DomainMap `domains`.
	 */
	static DomainVector project(const DomainMap& domains, const VariableIdxVector& scope);
	
	
	//! Deep-copies a domain map
	static DomainMap clone(const DomainMap& domains);
	
	//! Helper to print sets of domains
	static void printDomain(const Domain& domain);
	static void printDomains(const DomainMap& domains);
	static void printDomains(const DomainVector& domains);
};


} // namespaces
