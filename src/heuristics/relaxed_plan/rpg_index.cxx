
#include <heuristics/relaxed_plan/rpg_index.hxx>
#include <aptk2/tools/logging.hxx>
#include <utils/tuple_index.hxx>
#include <state.hxx>
#include <actions/actions.hxx>
#include <actions/action_id.hxx>
#include <problem_info.hxx>
#include <constraints/gecode/extensions.hxx>

namespace fs0 { namespace gecode {


RPGIndex::RPGIndex(const State& seed, const AtomIndex& tuple_index, ExtensionHandler& extension_handler) :
	_reached(tuple_index.size(), nullptr),
	_novel_tuples(),
	_current_layer(0),
	_extension_handler(extension_handler),
	_tuple_index(tuple_index),
	_seed(seed)
{
	_domains.reserve(seed.numAtoms());
	_extension_handler.reset();
	
	_domains_raw.resize(seed.numAtoms());
	
	// Initially we insert the seed state atoms
	for (unsigned variable = 0; variable < seed.numAtoms(); ++variable) {
		ObjectIdx value = seed.getValue(variable);
		
		AtomIdx tuple_index = _extension_handler.process_atom(variable, value);
		if (tuple_index != INVALID_TUPLE) {
			_domains.push_back(Gecode::IntSet(value, value));
			add(tuple_index, nullptr, {});
			
		} else {
			_domains.push_back(Gecode::IntSet()); // We simply push an empty domain for those predicative state variables that are set to false.
		}
	}
	next();
}

void RPGIndex::advance() {
	_extension_handler.advance();
	
	for (AtomIdx tuple:_novel_tuples) {
		_extension_handler.process_tuple(tuple);
	}
	
	// Now update the per-variable domains
	for (unsigned variable = 0; variable < _domains_raw.size(); ++variable) {
		const auto& all = _domains_raw[variable];
		// An intermediate IntArgs object seems to be necessary, since IntSets do not accept std-like range constructors.
		_domains[variable] = Gecode::IntSet(Gecode::IntArgs(all.cbegin(), all.cend()));
	}
	
	next();
}

void RPGIndex::next() {
	_extensions = _extension_handler.generate_extensions();
	_novel_tuples.clear();
	++_current_layer;
}


RPGIndex::~RPGIndex() {
	// delete all the pointers to action IDs, which belong to this container
	for (auto element:_reached) {
		if (element != nullptr) {
			const ActionID* action_id = std::get<1>(*element);
			delete action_id;
			delete element;
		}
	}
}


RPGIndex::TupleSupport* RPGIndex::createTupleSupport(const ActionID* action, std::vector<AtomIdx>&& support) const {
	return new TupleSupport(_current_layer, action, std::move(support));
}

const RPGIndex::TupleSupport& RPGIndex::getTupleSupport(AtomIdx tuple) const {
	auto support = _reached.at(tuple);
	assert(support != nullptr);
	return *support;
}

bool RPGIndex::reached(AtomIdx tuple) const {
	return _reached.at(tuple) != nullptr;
}

void RPGIndex::add(AtomIdx tuple, const ActionID* action, std::vector<AtomIdx>&& support) {
	auto& it = _reached.at(tuple);
	if (it != nullptr) return; // Don't insert the atom if it was already tracked by the RPG
	it = createTupleSupport(action, std::move(support)); // This effectively inserts the tuple into '_reached'
	_novel_tuples.push_back(tuple);
	const Atom& atom = _tuple_index.to_atom(tuple);
	auto& domain = _domains_raw.at(atom.getVariable());
	// assert(std::find(domain.cbegin(), domain.cend(), atom.getValue()) == domain.end()); // Warning: this is expensive
	domain.push_back(atom.getValue());
}

/*
unsigned RPGIndex::compute_hmax_sum(const std::vector<Atom>& atoms) const {
	unsigned sum = 0;
	for (const Atom& atom:atoms) {
		sum += std::get<0>(getTupleSupport(atom));
	}
	return sum;
}
*/

std::ostream& RPGIndex::print(std::ostream& os) const {
	const ProblemInfo& info = ProblemInfo::getInstance();
	os << "RPG Tuples: " << std::endl;
	for (unsigned i = 0; i < _reached.size(); ++i) {
		TupleSupport* support = _reached[i];
		if (support != nullptr) {
			const ActionID* action_id = std::get<1>(*support);
			os << "Tuple: " << i  << "\t(Atom: " << _tuple_index.to_atom(i) << ")\t- action: ";
			(action_id ? os << *action_id : os << "[INVALID-ACTION]");
			os << "\t- layer #" << std::get<0>(*support) << " - support: ";
			printAtoms(std::get<2>(*support), os);
			os << std::endl;
		}
	}
	os << std::endl;
	
	os << "RPG State Variable Domains: " << std::endl;
	for (unsigned variable = 0; variable < _domains.size(); ++variable) {
		os << info.getVariableName(variable) << ": " << _domains[variable] << std::endl;
	}
	
	return os;
}

void RPGIndex::printAtoms(const std::vector<AtomIdx>& vector, std::ostream& os) const {
	for (const auto& element:vector) {
		os << element << ", ";
	}
}


bool RPGIndex::is_true(VariableIdx variable) const {
	const auto& domain = _domains_raw.at(variable);
	 
	assert(domain.size() <= 1); // The variable must be predicative, thus will contain at most the element true
	assert(domain.empty() || domain[0] == 1); // If there is one element, it must be the True element
	return !domain.empty();
// 	return std::find(domain.cbegin(), domain.cend(), 1) != domain.end();
}

std::vector<bool> RPGIndex::achieved_atoms(const AtomIndex& tuple_index) const {
	std::vector<bool> achieved(tuple_index.size(), false);
	for (AtomIdx tuple = 0; tuple < tuple_index.size(); ++tuple) {
// 		if (!reached(tuple)) unachieved.insert(tuple);
		if (reached(tuple)) achieved[tuple] = true;
	}
	return achieved;
}

/*
const std::set<unsigned>& RPGIndex::get_modified_symbols() const {
	return _extension_handler.get_modified_symbols();
}
*/

} } // namespaces

