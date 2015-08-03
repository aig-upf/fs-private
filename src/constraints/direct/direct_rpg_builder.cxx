

#include <constraints/direct/direct_rpg_builder.hxx>
#include <utils/utils.hxx>
#include <utils/projections.hxx>
#include <problem.hxx>
#include <constraints/direct/constraint.hxx>
#include <constraints/direct/compiled.hxx>
#include <constraints/direct/translators/translator.hxx>
#include <utils/logging.hxx>

namespace fs0 {

DirectRPGBuilder::cptr DirectRPGBuilder::create(const std::vector<fs::AtomicFormula::cptr>& goalConditions, const std::vector<fs::AtomicFormula::cptr>& stateConstraints) {
	auto directStateConstraints = DirectTranslator::generate(stateConstraints);
	ConstraintCompiler::compileConstraints(directStateConstraints);
	
	auto directGoalConstraints = DirectTranslator::generate(goalConditions);
	ConstraintCompiler::compileConstraints(directGoalConstraints);
	
	auto allGoalConstraints = Utils::merge(directGoalConstraints, directStateConstraints);
	return new DirectRPGBuilder(std::move(allGoalConstraints), std::move(directStateConstraints));
}
	
// Note that we use both types of constraints as goal constraints
DirectRPGBuilder::DirectRPGBuilder(const std::vector<DirectConstraint::cptr>&& goalConstraints, const std::vector<DirectConstraint::cptr>&& stateConstraints) :
	_stateConstraints(stateConstraints),
	_allGoalConstraints(goalConstraints), // Goal constraints include state constraints as well
	_stateConstraintsHandler(_stateConstraints),
	_goalConstraintsHandler(_allGoalConstraints) // We store all the constraints in a new vector so that we can pass a const reference 
	                                            // - we're strongly interested in the ConstraintManager having only a reference, not the actual value,
	                                            // since in some cases each grounded action will have a ConstraintManager
{
}

DirectRPGBuilder::~DirectRPGBuilder() {
	for (const auto ptr: _allGoalConstraints) delete ptr;
	// We do not need to delete the state constraints, as they form part of the goal constraint vector as well
}

FilteringOutput DirectRPGBuilder::pruneUsingStateConstraints(RelaxedState& state) const {
	if (_stateConstraints.empty()) return FilteringOutput::Unpruned;
	DomainMap domains = Projections::project(state, _stateConstraintsHandler.getAllRelevantVariables());  // This does NOT copy the domains
	return _stateConstraintsHandler.filter(domains);
}

bool DirectRPGBuilder::isGoal(const State& seed, const RelaxedState& state, Atom::vctr& causes) const {
	assert(causes.empty());
	DomainMap domains = Projections::projectCopy(state, _goalConstraintsHandler.getAllRelevantVariables());  // This makes a copy of the domain.
	if (!checkGoal(domains)) return false;
	
	unsigned numVariables = Problem::getCurrentProblem()->getProblemInfo().getNumVariables(); // The total number of state variables
	std::vector<bool> set(numVariables, false); // Variables that have been already set.
	
	DomainMap clone = Projections::clone(domains);  // We need a deep copy

	extractGoalCauses(seed, domains, clone, causes, set, 0);
	// reportGoalFound(state, domains, causes);
	return true;
}

bool DirectRPGBuilder::isGoal(const RelaxedState& state) const {
	DomainMap domains = Projections::projectCopy(state, _goalConstraintsHandler.getAllRelevantVariables());  // This makes a copy of the domain.
	return checkGoal(domains);
}

bool DirectRPGBuilder::checkGoal(const DomainMap& domains) const {
	FilteringOutput o = _goalConstraintsHandler.filter(domains);
	return o != FilteringOutput::Failure && DirectCSPHandler::checkConsistency(domains);
}

//! Note that here we can guarantee that we'll always insert different atoms in `causes`, since all the inserted atoms have a different variable
void DirectRPGBuilder::extractGoalCauses(const State& seed, const DomainMap& domains, const DomainMap& clone, Atom::vctr& causes, std::vector<bool>& set, unsigned num_set) const {
	
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
	FilteringOutput o = _goalConstraintsHandler.filter(domains);
	if (o == FilteringOutput::Failure || !DirectCSPHandler::checkConsistency(domains)) {
		// If the selection made the domains inconsistent, instead of backtracking we simply
		// select arbitrary domains from the original domain set.
		extractGoalCausesArbitrarily(seed, clone, causes, set);
	} else{
		// Otherwise we keep propagating
		extractGoalCauses(seed, domains, clone, causes, set, num_set+1);
	}
}

void DirectRPGBuilder::extractGoalCausesArbitrarily(const State& seed, const DomainMap& domains, Atom::vctr& causes, std::vector<bool>& set) const {
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

std::ostream& DirectRPGBuilder::print(std::ostream& os) const {
	os << "[Basic Goal Constraint Manager]";
	return os;
}

} // namespaces

