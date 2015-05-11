
#include <relaxed_effect_manager.hxx>
#include <atoms.hxx>
#include <utils/cartesian_product.hxx>

namespace fs0 {

void RelaxedEffectManager::computeChangeset(const Action& action, const DomainMap& domains, Changeset& changeset) const {
	
	for (const ScopedEffect::cptr effect:action.getEffects()) {
		
		const VariableIdxVector& relevant = effect->getScope();
		
		if(relevant.size() == 0) {  // No need to pass any point.
			changeset.add(Fact(effect->getAffected(), effect->apply()), {});
		}
		else if(relevant.size() == 1) {  // micro-optimization for unary effects
			VariableIdx rel = relevant[0];
			for (ObjectIdx val:*(domains.at(rel))) { // Add to the changeset for every allowed value of the relevant variable
				computeUnaryChangeset(effect, rel, val, changeset);
			}
		}
		else { // The general, n-ary case. We iterate over the cartesian product of the allowed values for the relevant variables.
			const DomainVector projection = Projections::project(domains, relevant);
			CartesianProductIterator it(projection);
			
			for (; !it.ended(); ++it) {
				computeNAryChangeset(effect, relevant, *it, changeset);
			}
		}
	}
}

// Micro-optimization for unary effects
void RelaxedEffectManager::computeUnaryChangeset(const ScopedEffect::cptr effect, VariableIdx relevant, ObjectIdx value, Changeset& changeset) const {
       
       // TODO - Note that this won't work for conditional effects where an action might have no effect at all
       VariableIdx affected = effect->getAffected();

       // Add as extra causes all the relevant facts of the effect procedure.
       changeset.add(Fact(affected, effect->apply(value)), {Fact(relevant, value)});
}




void RelaxedEffectManager::computeNAryChangeset(const ScopedEffect::cptr effect, const VariableIdxVector& relevant, const ObjectIdxVector& values, Changeset& changeset) const {
	
	// TODO - Note that this won't work for conditional effects where an action might have no effect at all
	VariableIdx affected = effect->getAffected();

	// Add as extra causes all the relevant facts of the effect procedure.
	Fact::vctr extraCauses;
	for (unsigned i = 0; i < relevant.size(); ++i) {
		extraCauses.push_back(Fact(relevant[i], values[i]));
	}
	changeset.add(Fact(affected, effect->apply(values)), extraCauses);
}



} // namespaces
