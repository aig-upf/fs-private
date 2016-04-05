
#include <heuristics/relaxed_plan/rpg_index.hxx>
#include <utils/logging.hxx>
#include <utils/tuple_index.hxx>
#include <state.hxx>
#include <actions/actions.hxx>
#include <actions/action_id.hxx>
#include <problem.hxx>
#include <constraints/gecode/extensions.hxx>

namespace fs0 { namespace gecode {


RPGIndex::RPGIndex(const State& seed, const TupleIndex& tuple_index, ExtensionHandler& extension_handler) :
	_reached(tuple_index.size(), nullptr),
	_novel_tuples(),
	_current_layer(1),
	_extension_handler(extension_handler),
	_tuple_index(tuple_index)
{
	_domains.reserve(seed.numAtoms());
	_domains_raw.reserve(seed.numAtoms());
	_extension_handler.reset();
	
	// Initially we insert the seed state atoms
	for (unsigned variable = 0; variable < seed.numAtoms(); ++variable) {
		ObjectIdx value = seed.getValue(variable);
		
		TupleIdx tuple_index = _extension_handler.process_atom(variable, value);
		if (tuple_index != INVALID_TUPLE) {
			_domains_raw.push_back({value});
			_domains.push_back(Gecode::IntSet(value, value));
			add(tuple_index, nullptr, std::vector<TupleIdx>());
			
		} else {
			_domains_raw.push_back(std::set<ObjectIdx>());
			_domains.push_back(Gecode::IntSet()); // We simply push an empty domain for those predicative state variables that are set to false.
		}
	}
	
	_extensions = _extension_handler.generate_extensions();
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


RPGIndex::TupleSupport* RPGIndex::createTupleSupport(const ActionID* action, std::vector<TupleIdx>&& support) const {
	return new TupleSupport(_current_layer, action, std::move(support));
}

const RPGIndex::TupleSupport& RPGIndex::getTupleSupport(TupleIdx tuple) const {
	auto support = _reached.at(tuple);
	assert(support != nullptr);
	return *support;
}

bool RPGIndex::reached(TupleIdx tuple) const {
	return _reached.at(tuple) != nullptr;
}

void RPGIndex::add(TupleIdx tuple, const ActionID* action, std::vector<TupleIdx>&& support) {
	auto& it = _reached.at(tuple);
	if (it != nullptr) return; // Don't insert the atom if it was already tracked by the RPG
	it = createTupleSupport(action, std::move(support));
	_novel_tuples.push_back(tuple);	
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
	const ProblemInfo& info = Problem::getInfo();
	os << "RPG Tuples: " << std::endl;
	for (unsigned i = 0; i < _reached.size(); ++i) {
		TupleSupport* support = _reached[i];
		if (support != nullptr) {
			const ActionID* action_id = std::get<1>(*support);
			os << "Tuple: " << i  << " (Atom: " << _tuple_index.to_atom(i) << ")" << " - action: ";
			(action_id ? os << *action_id : os << "[INVALID-ACTION]");
			os << " - layer #" << std::get<0>(*support) << " - support: ";
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

void RPGIndex::printAtoms(const std::vector<TupleIdx>& vector, std::ostream& os) const {
	for (const auto& element:vector) {
		os << element << ", ";
	}
}

void RPGIndex::advance() {
	
	_extension_handler.advance();
	
	std::vector<std::vector<ObjectIdx>> novel_values(_domains.size());
	for (TupleIdx tuple:_novel_tuples) {
		_extension_handler.process_tuple(tuple);
		const Atom& atom = _tuple_index.to_atom(tuple);
		_domains_raw[atom.getVariable()].insert(atom.getValue());
	}
	
	// Now update the per-variable domains
	for (unsigned variable = 0; variable < _domains_raw.size(); ++variable) {
		const auto& all = _domains_raw[variable];
		// An intermediate IntArgs object seems to be necessary, since IntSets do not accept std-like range constructors.
		_domains[variable] = Gecode::IntSet(Gecode::IntArgs(all.cbegin(), all.cend()));
	}
	
	_extensions = _extension_handler.generate_extensions();
	_novel_tuples.clear();
	++_current_layer;
}


bool RPGIndex::is_true(VariableIdx variable) const {
	const auto& domain = _domains_raw.at(variable);
	return domain.find(1) != domain.end();
}

} } // namespaces

