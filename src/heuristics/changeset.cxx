
#include <heuristics/changeset.hxx>
#include <heuristics/rpg.hxx>

namespace aptk { namespace core {
	
const Changeset::FactData Changeset::INVALID_ACHIEVER = std::tuple<ActionIdx, Fact::vctr, unsigned>(CoreAction::INVALID_ACTION, {}, 0);

void Changeset::add(const Fact& fact, const FactSetPtr extraCauses) {
	if (_referenceState.contains(fact)) { // We want to ensure that the fact is novel.
		return;
	}
	
	std::vector<Fact> extra = RPGraph::pruneSeedSupporters(extraCauses, _seed);
	assert(_activeActionNonSeedSupportAtoms >= 0);
	unsigned total_non_seed_supporters = _activeActionNonSeedSupportAtoms + extra.size();
	
	auto lb = _effects.lower_bound(fact); // @see http://stackoverflow.com/a/101980
	if(lb != _effects.end() && _effects.key_comp()(fact, lb->first)) { // The key is already on the map
		
		// If that is the case, the simple heuristic we follow is:
		// We only update the support for the fact if the new support set contains a smaller number of supports not present in the seed state.
		unsigned other_total_non_seed_supporters = std::get<2>(lb->second);
		if (total_non_seed_supporters < other_total_non_seed_supporters) { 
			lb->second = std::make_tuple(_activeActionIdx, std::move(extra), total_non_seed_supporters);
		}
	} else {
		_effects.insert(lb, std::make_pair(fact, std::make_tuple(_activeActionIdx, std::move(extra), total_non_seed_supporters)));
	}
}

} } // namespaces
