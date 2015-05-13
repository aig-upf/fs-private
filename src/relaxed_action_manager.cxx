
#include <cassert>
#include <iosfwd>

#include <relaxed_action_manager.hxx>
#include <utils/cartesian_product.hxx>
#include <constraints/constraint_manager.hxx>
#include <heuristics/rpg_data.hxx>
#include <heuristics/rpg.hxx>

namespace fs0 {

void ActionManagerFactory::instantiateActionManager(const Action::vcptr& actions) {
	for (const Action::cptr& action:actions) {
		BaseActionManager* manager = nullptr;
		bool action_has_nary_effects = checkActionHasNaryEffects(action);
		if (checkActionHasNaryPreconditions(action)) {
			manager = new GenericActionManager(*action, action_has_nary_effects);
		} else {
			manager = new UnaryActionManager(*action, action_has_nary_effects);
		}
		action->setConstraintManager(manager);
	}
}

bool ActionManagerFactory::checkActionHasNaryPreconditions(const Action::cptr action) {
	bool needs_generic_manager = false;
	for (const ScopedConstraint::cptr constraint:action->getConstraints()) {
		unsigned arity = constraint->getArity();
		if (arity == 0) {
			throw std::runtime_error("Static applicability procedure that should have been detected in compilation time");
		}
		else if(arity == 1) {
			if (!dynamic_cast<UnaryParametrizedScopedConstraint*>(constraint) || constraint->filteringType() != ScopedConstraint::Filtering::Unary) {
				throw std::runtime_error("Cannot handle this type of unary constraint in action preconditions.");
			}
		} else {
			needs_generic_manager = true;
		}
	}
	return needs_generic_manager;
}

bool ActionManagerFactory::checkActionHasNaryEffects(const Action::cptr action) {
	bool result = false;
	for (const ScopedEffect::cptr effect:action->getEffects()) {
		if (effect->getArity() > 1) result = true;
	}	
	return result;
}

	
void BaseActionManager::processAction(unsigned actionIdx, const Action& action, const State& seed, const RelaxedState& layer, RPGData& rpgData) const {
	// We compute the projection of the current relaxed state to the variables relevant to the action
	// Note that this _clones_ the actual domains, since we will next modify (prune) them.
	DomainMap actionProjection = Projections::projectToActionVariables(layer, action);
	
	// This prunes the domains using the constraints represented by each procedure.
	Fact::vctr causes;
	if (checkPreconditionApplicability(action, seed, actionProjection, causes)) { // If the action is applicable in the current RPG layer...
		Fact::vctrp actionSupport = RPGraph::pruneSeedSupporters(causes, seed);
		// We update the RPG data with all new reachable effects.
		processEffects(actionIdx, action, actionSupport ,seed, actionProjection, rpgData);
	}
}


void BaseActionManager::processEffects(unsigned actionIdx, const Action& action, Fact::vctrp actionSupport, const State& seed, const DomainMap& actionProjection, RPGData& rpgData) const {
	
	for (const ScopedEffect::cptr effect:action.getEffects()) {
		const VariableIdxVector& relevant = effect->getScope();
		
		if(relevant.size() == 0) {  // No need to pass any point.
			rpgData.add(Fact(effect->getAffected(), effect->apply()), actionIdx, actionSupport);
		}
		
		else if(relevant.size() == 1) {  // Micro-optimization for unary effects
			for (ObjectIdx value:*(actionProjection.at(relevant[0]))) { // Add to the RPG for every allowed value of the relevant variable
				// Add the relevant variable value to the atoms that made the action applicable. Note that this is slightly redundant since that same variable might have already a different value.
				// TODO To be corrected
				Fact::vctrp allCauses = std::make_shared<Fact::vctr>(*actionSupport);
				if (seed.getValue(relevant[0]) != value) {
					allCauses->push_back(Fact(relevant[0], value));
				}
				
				// TODO - Note that this won't work for conditional effects where an action might have no effect at all
				rpgData.add(Fact(effect->getAffected(), effect->apply(value)), actionIdx, allCauses);
			}
		}
		
		
		else { // The general, n-ary case. We iterate over the cartesian product of the allowed values for the relevant variables.
			const DomainVector effectProjection = Projections::project(actionProjection, relevant);
			CartesianProductIterator it(effectProjection);
			
			for (; !it.ended(); ++it) {
				
				const ObjectIdxVector& values = *it;

				// Add as extra causes all the relevant facts of the effect procedure.
				Fact::vctrp allCauses = std::make_shared<Fact::vctr>(*actionSupport);
				for (unsigned i = 0; i < relevant.size(); ++i) {
					if (seed.getValue(relevant[i]) != values[i]) {
						allCauses->push_back(Fact(relevant[i], values[i]));
					}
				}
				// TODO - Again this won't work for conditional effects where an action might have no effect at all
				rpgData.add(Fact(effect->getAffected(), effect->apply(values)), actionIdx, allCauses);
			}
		}
	}
}


bool UnaryActionManager::checkPreconditionApplicability(const Action& action, const State& seed, const DomainMap& domains, Fact::vctr& causes) const {
	for (const ScopedConstraint::cptr constraint:action.getConstraints()) {
		if (!isProcedureApplicable(constraint, domains, seed, causes)) return false;
	}
	return true;
}

bool UnaryActionManager::isProcedureApplicable(const ScopedConstraint::cptr constraint, const DomainMap& domains, const State& seed, Fact::vctr& causes) {
	// TODO - Optimize taking into account that the constraint is unary
	
	const VariableIdxVector& relevant = constraint->getScope();
	VariableIdx variable = relevant[0];
	bool cause_found = false;
	
	// Check first if the original value is applicable. This is slightly inefficient, but ensures that if the original values
	// make an action applicable, we won't waste time tracing the causes of other values that might make it applicable as well.
	const ObjectIdx value = seed.getValue(variable);
	if (constraint->isSatisfied(value)) {
		causes.push_back(Fact(variable, value));
		cause_found = true;
	}
	
	Domain& values = *(domains.at(variable));
	Domain new_domain;
	
	for (auto& value:values) {
		if (constraint->isSatisfied(value)) {
			if (!cause_found) { // We only want to insert one single (arbitrarily chosen) cause for the unary app constraint.
				causes.push_back(Fact(variable, value));
				cause_found = true;
			}
			new_domain.insert(new_domain.end(), value); // We'll insert the element at the end, since we're iterating in order.
		}
	}
	
	values = new_domain; // Update the values with those that satisfy the unary constraint.
	return cause_found;
}


bool GenericActionManager::checkPreconditionApplicability(const Action& action, const State& seed, const DomainMap& domains, Fact::vctr& causes) const {
	ScopedConstraint::Output o = manager.filter(domains);
	
	if(o == ScopedConstraint::Output::Failure || !ConstraintManager::checkConsistency(domains)) {
		return false;
	}
	
	// The CSP is _not_ inconsistent. To build the cause of applicability of the action, we pick arbitrary values among the remaining consistent values.
	// We favor the seed value, if it is among them.
	for (const auto& domain:domains) {
		VariableIdx variable = domain.first;
		
		ObjectIdx seed_value = seed.getValue(variable);
		if (domain.second->find(seed_value) == domain.second->end()) {  // If the original value makes the situation a goal, then we don't need to add anything for this variable.
			ObjectIdx value = *(domain.second->cbegin());
			causes.push_back(Fact(variable, value)); // Otherwise we simply select an arbitrary value.
		}
	}
	return true;
}

} // namespaces

