
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <utils/logging.hxx>
#include <state.hxx>
#include <actions/ground_action.hxx>

namespace fs0 {

RPGData::RPGData(const State& seed) :
	_novel(seed.numAtoms()),
	_num_novel(0),
	_current_layer(0),
	_effects()
{
	// Initially we insert the seed state atoms
	for (unsigned variable = 0; variable < seed.numAtoms(); ++variable) {
		ObjectIdx value = seed.getValue(variable);
		_effects.insert(std::make_pair(Atom(variable, value),
						createAtomSupport(GroundAction::invalid_action_id, std::make_shared<std::vector<Atom>>())));
	}
	FFDEBUG("heuristic", "RPG Layer #" << getCurrentLayerIdx() << ": " << *this);
	advanceLayer();
}

void RPGData::advanceLayer() {
	_num_novel= 0;
	_novel = std::vector<std::vector<ObjectIdx>>(_novel.size()); // completely clear the vector of novel atoms
	++_current_layer;
}

RPGData::AtomSupport RPGData::createAtomSupport(ActionIdx action, Atom::vctrp support) const {
	return std::make_tuple(_current_layer, action, support);
}

const RPGData::AtomSupport& RPGData::getAtomSupport(const Atom& atom) const {
	auto it = _effects.find(atom);
	assert(it != _effects.end());
	return it->second;
}

std::pair<bool, RPGData::SupportMap::iterator> RPGData::getInsertionHint(const Atom& atom) {
	SupportMap::iterator lb = _effects.lower_bound(atom); // @see http://stackoverflow.com/a/101980
	bool keyInMap = lb != _effects.end() && !(_effects.key_comp()(atom, lb->first));
	if (keyInMap) return std::make_pair(false, lb);
	else return std::make_pair(true, lb);
}

void RPGData::add(const Atom& atom, ActionIdx action, Atom::vctrp support, SupportMap::iterator hint) {
	_effects.insert(hint, std::make_pair(atom, createAtomSupport(action, support)));
	_novel[atom.getVariable()].push_back(atom.getValue());
	++_num_novel;
}

void RPGData::add(const Atom& atom, ActionIdx action, Atom::vctrp support) {
	auto hint = getInsertionHint(atom);
	if (!hint.first) return; // Don't insert the atom if it was already tracked by the RPG
	add(atom, action, support, hint.second);
}

unsigned RPGData::compute_hmax_sum(const std::vector<Atom>& atoms) const {
	unsigned sum = 0;
	for (const Atom& atom:atoms) {
		sum += std::get<0>(getAtomSupport(atom));
	}
	return sum;
}

std::ostream& RPGData::print(std::ostream& os) const {
	os << "Relaxed Planning Graph atoms (" << _effects.size() << "): " << std::endl;
	for (const auto& x:_effects) {
		os << x.first  << " - action #" << std::get<1>(x.second) << " - layer #" << std::get<0>(x.second) << " - support: ";
		printAtoms(std::get<2>(x.second), os);
		os << std::endl;
	}
	os << std::endl;
	return os;
}

void RPGData::printAtoms(const Atom::vctrp vector, std::ostream& os) const {
	for (const auto& fact:*vector) {
		os << fact << ", ";
	}
}

} // namespaces

