
#include <heuristics/changeset.hxx>
#include <heuristics/rpg.hxx>

namespace fs0 {
	
const Changeset::FactData Changeset::INVALID_ACHIEVER = std::pair<ActionIdx, Fact::vctrp>(Action::INVALID, std::make_shared<Fact::vctr>());

void Changeset::add(const Fact& fact, const Fact::vctr& extraCauses) {
	if (_referenceState.contains(fact)) { // We want to ensure that the fact is novel.
		return;
	}
	updateEffectMapSimple(fact, RPGraph::pruneSeedSupporters(extraCauses, _seed));
}

// void Changeset::updateEffectMap(const Fact& fact, const Fact::vctrp extra) {
// 	assert(_activeActionNonSeedSupportAtoms >= 0);
// 	unsigned total_non_seed_supporters = _activeActionNonSeedSupportAtoms + extra->size();	
// 	auto lb = _effects.lower_bound(fact); // @see http://stackoverflow.com/a/101980
//  	if(lb != _effects.end() && !(_effects.key_comp()(fact, lb->first))) { // The key is already on the map
// 		// If that is the case, the simple heuristic we follow is:
// 		// We only update the support for the fact if the new support set contains a smaller number of supports not present in the seed state.
// 		unsigned other_total_non_seed_supporters = std::get<2>(lb->second);
// 		if (total_non_seed_supporters < other_total_non_seed_supporters) { 
// 			lb->second = std::make_tuple(_activeActionIdx, extra, total_non_seed_supporters);
// 		}
// 	} else {
// 		_effects.insert(lb, std::make_pair(fact, std::make_tuple(_activeActionIdx, extra, total_non_seed_supporters)));
// 	}
// }

void Changeset::updateEffectMapSimple(const Fact& fact, const Fact::vctrp extra) {
	_effects[fact] = { _activeActionIdx, extra };
}

} // namespaces
