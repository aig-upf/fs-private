
#pragma once

#include <cassert>
#include <iosfwd>
#include <actions.hxx>
#include <core_types.hxx>
#include <constraints/constraints.hxx>

namespace aptk { namespace core {


/**
 * A basic goal-checking-as-CSP solver that currently only enforces arc consistency
 * on the variable domains, using the AC-3 algorithm (http://en.wikipedia.org/wiki/AC-3_algorithm).
 * Currently only binary constraints are supported.
 */
class CSPSolver
{
protected:
	
	const ApplicableEntity& _goal;
	
	const std::vector<Constraint::cptr>& _constraints;
	
public:
	//! An arc is a pair <procedure_id, variable_id>
	typedef std::pair<unsigned, VariableIdx> Arc;
	typedef boost::container::flat_set<Arc> ArcSet;
	
	CSPSolver(const ApplicableEntity& goal, const std::vector<Constraint::cptr>& constraints)
		: _goal(goal), _constraints(constraints) {}
	
	//! In the preprocessing phase we handle unary constraints and create the list of arcs to be processed by the AC3 algorithm
	void preprocess(DomainSet& domains, std::vector<Arc>& arcs) const {
		for (unsigned proc = 0; proc < _goal.getNumApplicabilityProcedures(); ++proc) {
			const VariableIdxVector& relevant = _goal.getApplicabilityRelevantVars(proc);
			
			assert(relevant.size() <= 2); // At the moment we do not support higher-arity constraints
			
			if (relevant.size() == 1) {
				// For unary constraints, we simply prune the respective domains
				filter_unary_constraint(proc, domains);
			} else { 
				// For binary constraints, we prepare the necessary data for applying the AC3 algorithm
				arcs.push_back(std::make_pair(proc, relevant[0]));
				arcs.push_back(std::make_pair(proc, relevant[1]));
			}
		}
	}
	
	//! Filtering for unary constraints - filters the domain related to the unary constraint identified by `procedureIdx`
	void filter_unary_constraint(unsigned procedureIdx, DomainSet& domains) const {
		// We simply iterate through the domain values, checking them one by one and discarding them if not applicable
		VariableIdx relevant = _goal.getApplicabilityRelevantVars(procedureIdx)[0];
		DomainSetVector new_domain;
		const DomainSetVector& old_domain = domains[relevant];
		for (ObjectIdx obj:old_domain) {
			if (_goal.isApplicable(procedureIdx, {obj})) {
				new_domain.insert(new_domain.cend(), obj); // We will insert on the end of the container, as it is already sorted.
			}
		}
		domains[relevant] = new_domain;  // Update the domain with the new values // TODO - USE POINTERS?
	}
	
	//! 
	Constraint::Output filter_binary_constraints(DomainSet& domains, ArcSet& worklist) const {
		bool change = false;
		
		//2. Analyse pending arcs until the worklist is empty
		while (!worklist.empty()) {
			Arc a = select(worklist);
			const auto& procedure = a.first;
			const auto& X = a.second;
			const VariableIdxVector& rel = _goal.getApplicabilityRelevantVars(procedure);
			
			// 3. Arc-reduce the procedure `procedure` with respect to the variable X 
			DomainSetVector new_domain;
			
			
			// This is a bit hacky, but performant
			if (rel[0] == X) {
				for (auto x:domains[X]) {
					for (auto z:domains[rel[1]]) {
						if (_goal.isApplicable(procedure, {x, z})) {
							new_domain.insert(new_domain.cend(), x); // We will insert on the end of the container, as it is already sorted.
							break; // Break the inner loop only
						}
					}
				}
			}  else if (rel[1] == X) {
				for (auto x:domains[X]) {
					for (auto z:domains[rel[0]]) {
						if (_goal.isApplicable(procedure, {z, x})) {
							new_domain.insert(new_domain.cend(), x); // We will insert on the end of the container, as it is already sorted.
							break; // Break the inner loop only
						}
					}
				}
			}
			
			if (new_domain.size() == 0) {
				return Constraint::Output::Failure;
			}
			
			// 4. If we have removed some element from the domain, we insert the related constraints into the worklist
			//    in order to reconsider them again.
			if (new_domain.size() != domains[X].size()) {
				domains[X] = new_domain; // TODO - USE POINTERS?
				
				for (unsigned proc_prime = 0; proc_prime < _goal.getNumApplicabilityProcedures(); ++proc_prime) {
					if (procedure == proc_prime) continue;
					const VariableIdxVector& rel = _goal.getApplicabilityRelevantVars(proc_prime);
					
					// Let Z be the relevant variable that is not X
					unsigned Z = 0;
					if (X == rel[0]) Z = rel[1];
					else if (X == rel[1]) Z = rel[0];
					else continue;
					
					worklist.insert(std::make_pair(proc_prime, Z));
				}
			}
		}
		
		if (change) {
			if (!check_consistency(domains)) return Constraint::Output::Failure;
			else return Constraint::Output::Pruned;
		} else {
			return Constraint::Output::Unpruned;
		}
	}
	
	//! Return true iff all variable domain are non-empty
	bool check_consistency(const DomainSet& domains) const {
		for (const auto& domain:domains) {
			if (domain.second.size() == 0) return false; // If any pruned domain is empty, the CSP has no solution.
		}
		return true;
	}
	
	Constraint::Output filter_global_constraints(DomainSet& domains) const {
		Constraint::Output output = Constraint::Output::Unpruned;
		for (auto constraint:_constraints) {
			Constraint::Output o = constraint->enforce_consistency(domains);
			if (o == Constraint::Output::Failure) return o;
			if (o == Constraint::Output::Pruned) output = o;
		}
		return output;
	}
	
	Constraint::Output enforce_consistency(DomainSet& domains) const {
		std::vector<Arc> tmp;
		preprocess(domains, tmp);
		if (!check_consistency(domains)) return Constraint::Output::Failure;
		
		Constraint::Output o = Constraint::Output::Unpruned;
		ArcSet worklist(tmp.begin(), tmp.end());
		
// 		printArcSet(worklist);
		
		// First apply both types of filtering
		o = filter_binary_constraints(domains, worklist);
		if (o == Constraint::Output::Failure) return o;
		
		o = filter_global_constraints(domains);
		if (o == Constraint::Output::Failure) return o;
		
		// Now, keep pruning until we reach a fixpoint.
		while (o == Constraint::Output::Pruned) {
			o = filter_binary_constraints(domains, worklist);
			
			if (o == Constraint::Output::Pruned) {
				o = filter_global_constraints(domains);
			}
		}
		return o;
	}
	
// 	void enforce_consistency(DomainSet& domains) {
// 		std::vector<Arc> tmp;
// 		preprocess(domains, tmp);
// 		ArcSet worklist(tmp.begin(), tmp.end());
// 		
// 		// Meaning "need to apply binary filter", and "global filter", respectively.
// 		bool filter_binary = true, filter_global = true;
// 
// 		// Apply both types of filtering successively until a fixpoint is reached.
// 		do {
// 			// If binary filtering prunes any value, we need to apply global filtering again
// 			if (filter_binary) {
// 				if (filter_binary_constraints(domains, worklist)) {
// 					filter_global = true;
// 				}
// 				filter_binary = false;
// 			}
// 			
// 			// If global filtering prunes any value, we need to apply binary filtering again
// 			if (filter_global) {
// 				if (filter_global_constraints(domains)) {
// 					filter_binary = true;
// 				}
// 				filter_global = false;
// 			}
// 		} while (filter_binary || filter_global);
// 	}

	void printArcSet(const ArcSet& arcs) const {
		const auto problemInfo = Problem::getCurrentProblem()->getProblemInfo();
		std::cout << "arc-set: " << std::endl;
		for (const auto& arc:arcs) {
			std::cout << "["  << arc.first << ", " << problemInfo->getVariableName(arc.second) << "]" << std::endl;
		}
	}
	
	//! Helper to print the current domains
	static void printDomains(const DomainSet& domains) {
		const auto problemInfo = Problem::getCurrentProblem()->getProblemInfo();
 	 	for (const auto& domain:domains) {
			std::cout << problemInfo->getVariableName(domain.first) << "={";
			for (auto objIdx:domain.second) {
				std::cout << problemInfo->getObjectName(domain.first, objIdx) << ",";
			}
 	 		std::cout << "}" << std::endl;
		}
	}	

protected:
	
	//! We select an arbitrary arc, indeed the first according to the order between pairs of procedure IDs and variable IDs.
	//! and remove it from the worklist
	Arc select(ArcSet& worklist) const {
		assert(!worklist.empty());
		const auto& it = worklist.end() - 1;
		auto elem = *(it);
		worklist.erase(it);
		return elem;
	}
};



} } // namespaces

