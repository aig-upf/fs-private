
#include <cassert>
#include <iosfwd>

#include <relaxed_action_manager.hxx>
#include <complex_action_manager.hxx>
#include <utils/cartesian_product.hxx>
#include <constraints/constraint_manager.hxx>
#include <heuristics/rpg_data.hxx>
#include <heuristics/rpg.hxx>

#include <utils/logging.hxx>

namespace fs0 {

void ActionManagerFactory::instantiateActionManager(const Problem& problem, const Action::vcptr& actions) {

	for (const Action::cptr& action:actions) {
		BaseActionManager* manager = nullptr;

		bool complexPreconditions = actionHasHigherArityPreconditions(action);
		bool complexEffects = actionHasHigherArityEffects(action);

		if ( true ) { //complexPreconditions || complexEffects) {
			manager = new ComplexActionManager(*action, problem.getConstraints());
			FDEBUG("main", "Generated CSP for action " << *action << std::endl <<  *manager << std::endl);
		} else {
			manager = new UnaryActionManager(*action);
		}

		action->setConstraintManager(manager);
	}
}

bool ActionManagerFactory::actionHasHigherArityPreconditions(const Action::cptr action) {
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

bool ActionManagerFactory::actionHasHigherArityEffects(const Action::cptr action) {
	bool result = false;
	for (const ScopedEffect::cptr effect:action->getEffects()) {
		if (effect->getArity() == 0 && !effect->applicable()) {
			throw std::runtime_error("A 0-ary non-applicable effect was detected.");
		}
		if (effect->getArity() > 1) result = true;
	}
	return result;
}


void UnaryActionManager::processAction(unsigned actionIdx, const Action& action, const RelaxedState& layer, RPGData& rpg) {
	// We compute the projection of the current relaxed state to the variables relevant to the action
	// Note that this _clones_ the actual domains, since we will next modify (prune) them.
	DomainMap actionProjection = Projections::projectCopyToActionVariables(layer, action);

	if (checkPreconditionApplicability(actionProjection)) { // Check with local consistency
		processEffects(actionIdx, action, actionProjection, rpg);
	}
}

bool UnaryActionManager::checkPreconditionApplicability(const DomainMap& domains) const {
	ScopedConstraint::Output o = manager.filter(domains);
	return o != ScopedConstraint::Output::Failure && ConstraintManager::checkConsistency(domains);
}

void UnaryActionManager::processEffects(unsigned actionIdx, const Action& action, const DomainMap& actionProjection, RPGData& rpg) const {
	const VariableIdxVector& actionScope = action.getScope();
	FFDEBUG("main", "processing action effects: " << action.getName());

	for (const ScopedEffect::cptr effect:action.getEffects()) {
		const VariableIdxVector& effectScope = effect->getScope();
		FFDEBUG("main", "\t effect: " << effect);

		/***** 0-ary Effects *****/
		if(effectScope.size() == 0) {  // No need to pass any point.
			assert(effect->applicable()); // The effect is assumed to be applicable - non-applicable 0-ary effects make no sense and are detected before the search.
			FFDEBUG("main", "\t\t 0-ary effect");
			Atom atom = effect->apply();
			auto hint = rpg.getInsertionHint(atom);

			if (hint.first) {
				Atom::vctrp support = std::make_shared<Atom::vctr>();
				completeAtomSupport(actionScope, actionProjection, effectScope, support);
				rpg.add(atom, actionIdx, support, hint.second);
			}
		}

		/***** Unary Effects *****/
		else if(effectScope.size() == 1) {  // Micro-optimization for unary effects
			FFDEBUG("main", "\t\t 1-ary effect");
			for (ObjectIdx value:*(actionProjection.at(effectScope[0]))) { // Add to the RPG for every allowed value of the relevant variable
				if (!effect->applicable(value)) continue;
				Atom atom = effect->apply(value);
				auto hint = rpg.getInsertionHint(atom);

				if (hint.first) {
					Atom::vctrp support = std::make_shared<Atom::vctr>();
					support->push_back(Atom(effectScope[0], value));// Just insert the only value
					completeAtomSupport(actionScope, actionProjection, effectScope, support);
					rpg.add(atom, actionIdx, support, hint.second);
				}
			}
		}

		/***** Higher-arity Effects *****/
		else { // The general, n-ary case. We iterate over the cartesian product of the allowed values for the relevant variables.
			throw std::runtime_error("Shouldn't be here!");
			/*
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

				rpg.add(effect->apply(values), actionIdx, support);
			}
			*/
		}
	}
}

void UnaryActionManager::completeAtomSupport(const VariableIdxVector& actionScope, const DomainMap& actionProjection, const VariableIdxVector& effectScope, Atom::vctrp support) const {

	for (VariableIdx variable:actionScope) {
		if (effectScope.empty() || variable != effectScope[0]) { // (We know that the effect scope has at most one variable)
			ObjectIdx value = *(actionProjection.at(variable)->cbegin());
			support->push_back(Atom(variable, value));
		}
	}
}


std::ostream& UnaryActionManager::print(std::ostream& os) const {
	os << "UnaryActionManager[]";
	return os;
}

/*
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
	//DomainMap domains(actionProjection); // Clone the projection - this performs a certain amount of unnecessary work
	// MRJ: Copy the domains as well - the above only copies the pointers!
	std::vector<Domain> 	domainsStorage;
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
*/



} // namespaces
