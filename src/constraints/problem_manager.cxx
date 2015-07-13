

#include <constraints/problem_manager.hxx>
#include "gecode_constraint_manager.hxx"
#include <problem.hxx>
#include <utils/projections.hxx>
#include <utils/utils.hxx>
#include <utils/config.hxx>

namespace fs0 {

// Note that we use both types of constraints as goal constraints
PlanningConstraintManager::PlanningConstraintManager(const ScopedConstraint::vcptr& goalConstraints, const ScopedConstraint::vcptr& stateConstraints) :
	allGoalConstraints(Utils::merge(goalConstraints, stateConstraints)),
	stateConstraintsManager(stateConstraints),
	goalConstraintsManager(allGoalConstraints), // We store all the constraints in a new vector so that we can pass a const reference 
	                                            // - we're strongly interested in the ConstraintManager having only a reference, not the actual value,
	                                            // since in some cases each grounded action will have a ConstraintManager
	hasStateConstraints(stateConstraints.size() > 0)
{}

ScopedConstraint::Output PlanningConstraintManager::pruneUsingStateConstraints(RelaxedState& state) const {
	if (!hasStateConstraints) return ScopedConstraint::Output::Unpruned;
	DomainMap domains = Projections::project(state, stateConstraintsManager.getAllRelevantVariables());  // This does NOT copy the domains
	return stateConstraintsManager.filter(domains);
}

bool PlanningConstraintManager::isGoal(const State& seed, const RelaxedState& state, Atom::vctr& causes) const {
	assert(causes.empty());
	DomainMap domains = Projections::projectCopy(state, goalConstraintsManager.getAllRelevantVariables());  // This makes a copy of the domain.
	if (!checkGoal(domains)) return false;
	
	unsigned numVariables = Problem::getCurrentProblem()->getProblemInfo().getNumVariables(); // The total number of state variables
	std::vector<bool> set(numVariables, false); // Variables that have been already set.
	
	DomainMap clone = Projections::clone(domains);  // We need a deep copy

	extractGoalCauses(seed, domains, clone, causes, set, 0);
	// reportGoalFound(state, domains, causes);
	return true;
}

bool PlanningConstraintManager::isGoal(const RelaxedState& state) const {
	DomainMap domains = Projections::projectCopy(state, goalConstraintsManager.getAllRelevantVariables());  // This makes a copy of the domain.
	return checkGoal(domains);
}

bool PlanningConstraintManager::checkGoal(const DomainMap& domains) const {
	ScopedConstraint::Output o = goalConstraintsManager.filter(domains);
	return o != ScopedConstraint::Output::Failure && ConstraintManager::checkConsistency(domains);
}

//! Note that here we can guarantee that we'll always insert different atoms in `causes`, since all the inserted atoms have a different variable
void PlanningConstraintManager::extractGoalCauses(const State& seed, const DomainMap& domains, const DomainMap& clone, Atom::vctr& causes, std::vector<bool>& set, unsigned num_set) const {
	
	// 0. Base case
	if (num_set == domains.size()) return;
	
	VariableIdx selected_var = 0;
	unsigned min_domain_size = std::numeric_limits<unsigned>::max();
	DomainPtr selected_dom = nullptr;
	
	// 1. Select the variable with smallest domain that has not yet been set a value.
	for (auto& domain:domains) {
		VariableIdx variable = domain.first;
		if (set[variable]) continue;

		if (domain.second->size() < min_domain_size) {
			selected_var = variable;
			selected_dom = domain.second;
			min_domain_size = selected_dom->size();
		}
	}
	assert(selected_dom != nullptr);
	
	// 3. If the value that the variable had in the seed state is available, select it, otherwise select an arbitrary value
	ObjectIdx selected_value = seed.getValue(selected_var);
	if (selected_dom->find(selected_value) == selected_dom->end()) {
		selected_value = *(selected_dom->cbegin()); // We simply select an arbitrary value.
		assert(selected_var >= 0 && selected_value >= 0);
		causes.push_back(Atom(selected_var, selected_value)); // We only insert the fact if it wasn't true on the seed state.
	}
	set[selected_var] = true; // Tag the variable as already selected
	
	// 4 . Propagate the restrictions forward.
	// 4.1 Prune the domain.
	selected_dom->clear();
	selected_dom->insert(selected_value);
	
	// 4.2 Apply the constraints again
	ScopedConstraint::Output o = goalConstraintsManager.filter(domains);
	if (o == ScopedConstraint::Output::Failure || !ConstraintManager::checkConsistency(domains)) {
		// If the selection made the domains inconsistent, instead of backtracking we simply
		// select arbitrary domains from the original domain set.
		extractGoalCausesArbitrarily(seed, clone, causes, set);
	} else{
		// Otherwise we keep propagating
		extractGoalCauses(seed, domains, clone, causes, set, num_set+1);
	}
}

void PlanningConstraintManager::extractGoalCausesArbitrarily(const State& seed, const DomainMap& domains, Atom::vctr& causes, std::vector<bool>& set) const {
	for (const auto& domain:domains) {
		VariableIdx variable = domain.first;
		
		if (set[variable]) continue;
		set[variable] = true;
		
		ObjectIdx seed_value = seed.getValue(variable);
		if (domain.second->find(seed_value) == domain.second->end()) {  // If the original value makes the situation a goal, then we don't need to add anything for this variable.
			ObjectIdx value = *(domain.second->cbegin());
			causes.push_back(Atom(variable, value)); // Otherwise we simply select an arbitrary value.
		}
	}
}

BaseConstraintManager* PlanningConstraintManagerFactory::create(const ScopedConstraint::vcptr& goalConstraints, const ScopedConstraint::vcptr& stateConstraints) {
	const Config::GoalManagerType manager_t = Config::instance().getGoalManagerType();
	if (manager_t == Config::GoalManagerType::Gecode) {
		return new GecodeConstraintManager(goalConstraints, stateConstraints);
	} else {
		return new PlanningConstraintManager(goalConstraints, stateConstraints);
	}
}


} // namespaces

