
#pragma once

#include <cassert>
#include <iosfwd>
#include <actions.hxx>
#include <core_changeset.hxx>
#include <core_types.hxx>
#include <fact.hxx>
#include <core_problem.hxx>
#include <constraints/constraints.hxx>
#include <constraints/csp_solver.hxx>

namespace aptk { namespace core {


/**
 * A CSPGoalManager offers a way of checking if a relaxed state is a goal by modeling it as a CSP.
 */
class CSPGoalManager
{
protected:
	//! The goal constraints of the problem
	const ProblemConstraint::vctr& _gconstraints;
	
	//! The goal applicability entity
	const ApplicableEntity& _goal;
	
	CSPSolver _solver;
	
	
public:
	CSPGoalManager(const ApplicableEntity& goal, const ProblemConstraint::vctr& gconstraints) 
	: _gconstraints(gconstraints), _goal(goal), _solver(_goal, _gconstraints)
	
	{}
	
	//! Returns true iff the given RelaxedState is a goal according to the goal, state and goal constraints.
	bool isGoal(const State& seed, const RelaxedState& state) const {
		DomainSet domains;
		return checkGoal(seed, state, domains);
	}
	
	//! Returns true iff the given RelaxedState is a goal according to the goal, state and goal constraints.
	//! Besides, return the causes of the goal to be later processed by the RPG heuristic backchaining procedure.
	bool isGoal(const State& seed, const RelaxedState& state, FactSetPtr causes) const {
		
		DomainSet domains;
		if (!checkGoal(seed, state, domains)) {
			return false;
		}
		
		unsigned numVariables = Problem::getCurrentProblem()->getProblemInfo()->getNumVariables();
		std::vector<bool> set(numVariables, false); // Variables that have been already set.
 		extractGoalCauses(seed, domains, causes, set, 0);
		// reportGoalFound(state, domains, causes);
		return true;
	}
	
protected:
	//! 
	bool checkGoal(const State& seed, const RelaxedState& state, DomainSet& domains) const {
		// Extract all the relevant variable domains - including those of state constraints, which we'll use for goal checking.
		extractDomains(state, _gconstraints, domains);
 		// std::cout << "Domains BEFORE pruning:" << std::endl; CSPSolver::printDomains(domains);
		Constraint::Output o = _solver.enforce_consistency(domains);
		return o != Constraint::Output::Failure && _solver.check_consistency(domains);
	}
	
public:
	//! Extracts the domains of all the variables relevant to determine if a RPG layer is a goal.
	void extractDomains(const RelaxedState& state, const ProblemConstraint::vctr& constraints, DomainSet& domains) const {
		
		// Extract first the variables relevant for the goal procedures.
		for (unsigned proc = 0; proc < _goal.getNumApplicabilityProcedures(); ++proc) {
			for (const auto& variable: _goal.getApplicabilityRelevantVars(proc)) {
				extractDomain(state, variable, domains);
			}
		}
		
		// And then extract the variables relevant for the state constraints.
		for (const ProblemConstraint::cptr constraint:constraints) {
			for (VariableIdx variable:constraint->getScope()) {
				extractDomain(state, variable, domains);
			}
		}
	}
	
	//! Inserts in the DomainSet the values allowed for the given variable in the given relaxed state, if they were not there yet.
	static void extractDomain(const RelaxedState& state, VariableIdx variable, DomainSet& domains) {
		auto lb = domains.lower_bound(variable); // @see http://stackoverflow.com/a/101980
		if(lb == domains.end() || domains.key_comp()(variable, lb->first)) { // We have no associated values for relevant yet.
			
			const auto& value_set = state.getValues(variable);
			assert(value_set.size() != 0); // No domain should be empty
			
			// Use the lower bound as a hint for better performance
			domains.insert(lb, std::make_pair(variable, DomainSetVector(value_set.cbegin(), value_set.cend())));
		}
	}
	
	//! Extract the supporters of the goal from the pruned domains and add them to the set of goal causes.
	//! If any pruned domain is empty, return false, as it means we have an inconsistency.
	void extractGoalCauses(const State& seed, DomainSet& domains, FactSetPtr causes, std::vector<bool>& set, unsigned num_set) const {
		
		// 0. Base case
		if (num_set == domains.size()) return;
		
		VariableIdx selected_var = 0;
		unsigned min_domain_size = std::numeric_limits<unsigned>::max();
		DomainSetVector* selected_dom = NULL;
		
		// 1. Select the variable with smallest domain that has not yet been set a value.
		for (auto& domain:domains) {
			VariableIdx variable = domain.first;
			if (set[variable]) continue;

			if (domain.second.size() < min_domain_size) {
				selected_var = variable;
				selected_dom = &(domain.second);
				min_domain_size = selected_dom->size();
			}
 		}
 		assert(selected_dom != NULL);
		
		// 3. If the value that the variable had in the seed state is available, select it, otherwise select an arbitrary value
		ObjectIdx selected_value = seed.getValue(selected_var);
		if (selected_dom->find(selected_value) == selected_dom->end()) {
			selected_value = *(selected_dom->cbegin()); // We simply select an arbitrary value.
			causes->insert(Fact(selected_var, selected_value)); // We only insert the fact if it wasn't true on the seed state.
		}
		set[selected_var] = true; // Tag the variable as already selected
		
		// 4 . Propagate the restrictions forward.
		// 4.1 Prune the domain.
		selected_dom->clear();
		selected_dom->insert(selected_value);
		
		// 4.2 Apply the constraints again
		DomainSet copy(domains);
		Constraint::Output o = _solver.enforce_consistency(copy);
		if (o == Constraint::Output::Failure || !_solver.check_consistency(copy)) {
			// If the selection made the domains inconsistent, instead of backtracking we simply
			// select arbitrary domains from the original domain set.
			extractGoalCausesArbitrarily(seed, domains, causes, set);
		} else{
			// Otherwise we keep propagating
			extractGoalCauses(seed, copy, causes, set, num_set+1);
		}
	}

	void extractGoalCausesArbitrarily(const State& seed, const DomainSet& domains, FactSetPtr causes, std::vector<bool>& set) const {
		for (const auto& domain:domains) {
			VariableIdx variable = domain.first;
			
			if (set[variable]) continue;
			set[variable] = true;
			
			ObjectIdx seed_value = seed.getValue(variable);
			if (domain.second.find(seed_value) == domain.second.end()) {  // If the original value makes the situation a goal, then we don't need to add anything for this variable.
				ObjectIdx value = *(domain.second.cbegin());
				causes->insert(Fact(variable, value)); // Otherwise we simply select an arbitrary value.
			}
		}
	}
	
	
// 	//! Extract the supporters of the goal from the pruned domains and add them as causes in the justified entity.
// 	//! If any pruned domain is empty, return false, as it means we have an inconsistency.
// 	void extractGoalCausesNaively(const State& seed, const DomainSet& domains) {
// 		for (const auto& domain:domains) {
// 			ObjectIdx seed_value = seed.getValue(domain.first);
// 			if (domain.second.find(seed_value) == domain.second.end()) {  // If the original value makes the situation a goal, then we don't need to add anything for this variable.
// 				_justified_goal_entity.addCause(Fact(domain.first, *(domain.second.cbegin()))); // We simply add any combination of values of the domains.
// 			}
// 		}
// 	}	


	
	static void reportGoalFound(const RelaxedState& state, const DomainSet& domains, const FactSetPtr causes) {
		std::cout << std::endl << "GOAL FOUND!  RPG  layer:" << std::endl << std::endl;
		std::cout  << state;
		std::cout << std::endl << "PRUNED DOMAINS: " << std::endl;
		CSPSolver::printDomains(domains);
		
		std::cout << std::endl << "EXTRACTED CAUSES:" << std::endl;
		const auto problemInfo = Problem::getCurrentProblem()->getProblemInfo();
		for (const auto& cause:*causes) {
			std::cout << problemInfo->getVariableName(cause._variable) << " = ";
			std::cout << problemInfo->getObjectName(cause._variable, cause._value) << std::endl;
		}
	}
};

} } // namespaces

