
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
		if (effect->getArity() == 0 && !effect->applicable()) {
			throw std::runtime_error("A 0-ary non-applicable effect was detected.");
		}
		if (effect->getArity() > 1) result = true;
	}
	return result;
}


void BaseActionManager::processAction(unsigned actionIdx, const Action& action, const RelaxedState& layer, RPGData& rpgData) const {
	// We compute the projection of the current relaxed state to the variables relevant to the action
	// Note that this _clones_ the actual domains, since we will next modify (prune) them.
	DomainMap actionProjection = Projections::projectToActionVariables(layer, action);
	
	ScopedConstraint::Output o = manager.filter(actionProjection); // Check with local consistency
	if ( o != ScopedConstraint::Output::Failure && ConstraintManager::checkConsistency(actionProjection)) {
		processEffects(actionIdx, action, actionProjection, rpgData);
	}
}





void BaseActionManager::processEffects(unsigned actionIdx, const Action& action, const DomainMap& actionProjection, RPGData& rpgData) const {
	const VariableIdxVector& actionScope = action.getScope();
	#ifdef FS0_DEBUG
	std::cout << "processing action effects: " << action.getName() << std::endl;
	#endif

	for (const ScopedEffect::cptr effect:action.getEffects()) {
		const VariableIdxVector& effectScope = effect->getScope();
		#ifdef FS0_DEBUG
		std::cout << "\t effect: " << effect->getName() << std::endl;
		#endif
		
		/***** 0-ary Effects *****/
		if(effectScope.size() == 0) {  // No need to pass any point.
			assert(effect->applicable()); // The effect is assumed to be applicable - non-applicable 0-ary effects make no sense and are detected before the search.
			#ifdef FS0_DEBUG
			std::cout << "\t\t 0-ary effect" << std::endl;
			#endif	
			Atom atom = effect->apply();
			auto hint = rpgData.getInsertionHint(atom);
			
			if (hint.first) {
				Atom::vctrp support = std::make_shared<Atom::vctr>();
				completeAtomSupport(actionScope, actionProjection, effectScope, support);
				rpgData.add(atom, actionIdx, support, hint.second);
			}
		}
		
		/***** Unary Effects *****/
		else if(effectScope.size() == 1) {  // Micro-optimization for unary effects
			#ifdef FS0_DEBUG
			std::cout << "\t\t 1-ary effect" << std::endl;
			#endif	
			for (ObjectIdx value:*(actionProjection.at(effectScope[0]))) { // Add to the RPG for every allowed value of the relevant variable
				if (!effect->applicable(value)) continue;
				Atom atom = effect->apply(value);
				auto hint = rpgData.getInsertionHint(atom);
				
				if (hint.first) {
					Atom::vctrp support = std::make_shared<Atom::vctr>();
					support->push_back(Atom(effectScope[0], value));// Just insert the only value
					completeAtomSupport(actionScope, actionProjection, effectScope, support);
					rpgData.add(atom, actionIdx, support, hint.second);
				}
			}
		}
		
		/***** Higher-arity Effects *****/
		else { // The general, n-ary case. We iterate over the cartesian product of the allowed values for the relevant variables.
			const DomainVector effectProjection = Projections::project(actionProjection, effectScope);
			CartesianProductIterator it(effectProjection);
			#ifdef FS0_DEBUG
			std::cout << "\t\t n-ary effect" << std::endl;
			Projections::printDomains( effectProjection );
			#endif	
			for (; !it.ended(); ++it) {
				const ObjectIdxVector& values = *it;
				#ifdef FS0_DEBUG
				std::cout << "\t\t\tTesting applicability of effect:" << std::endl;
				std::cout << "\t\t\t";
				for ( auto value : values ) {
					std::cout << value << ",";
				}
				std::cout << std::endl;
				#endif	
				if (!effect->applicable(values)) continue; // Conditional effect check
				Atom::vctrp support = std::make_shared<Atom::vctr>();
				if (!isCartesianProductElementApplicable(actionScope, effectScope, actionProjection, values, support)) continue;
				
				rpgData.add(effect->apply(values), actionIdx, support);
			}
		}
	}
}

void BaseActionManager::completeAtomSupport(const VariableIdxVector& actionScope, const DomainMap& actionProjection, const VariableIdxVector& effectScope, Atom::vctrp support) const {
	boost::container::flat_set<VariableIdx> processed(effectScope.begin(), effectScope.end());
	for (VariableIdx variable:actionScope) {
		if (processed.find(variable) == processed.end()) {
			ObjectIdx value = *(actionProjection.at(variable)->cbegin());
			support->push_back(Atom(variable, value));
		}
	}
}


bool GenericActionManager::isCartesianProductElementApplicable(const VariableIdxVector& actionScope, const VariableIdxVector& effectScope, const DomainMap& actionProjection, const ObjectIdxVector& element, Atom::vctrp support) const {
	
	// We need to check that this concrete instantiation makes the action applicable - before we only checked for the local consistency of individual values
	DomainMap domains;
	
	// First fix the domains of the effect scope to the values of the cartesian product.
	for (unsigned i = 0; i < effectScope.size(); ++i) {
		const VariableIdx& variable = effectScope[i];
		const ObjectIdx& value = element[i];
		
		auto singleton = std::make_shared<Domain>();
		singleton->insert(value);
		domains.insert(std::make_pair(variable, singleton));
		
		support->push_back(Atom(variable, value)); // Zip the variables with their values		
	}
	
	// Now copy the rest of the domains
	for (auto elem: actionProjection) {
		// Insert a copy of the domain _only_ if we have not inserted a singleton domain on the previous iteration.
		auto lb = domains.lower_bound(elem.first); // see http://stackoverflow.com/a/101980
		
		if(lb == domains.end() || domains.key_comp()(elem.first, lb->first)) { // The key does not exist yet
			domains.insert(lb, std::make_pair(elem.first, std::make_shared<Domain>(*(elem.second))));   
		}
	}
	
	
	// And re-apply the filtering but with the domains of the variables relevant to the particular effect turned into singletons
	ScopedConstraint::Output o = manager.filter(domains);
	if (o == ScopedConstraint::Output::Failure) return false;
	
	completeAtomSupport(actionScope, domains, effectScope, support);
	return true;
}


bool BaseActionManager::checkPreconditionApplicability(const DomainMap& domains) const {
	ScopedConstraint::Output o = manager.filter(domains);
	return o != ScopedConstraint::Output::Failure && ConstraintManager::checkConsistency(domains);
}




} // namespaces

