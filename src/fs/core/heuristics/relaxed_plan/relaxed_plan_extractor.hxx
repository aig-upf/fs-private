
#pragma once

#include <vector>
#include <algorithm>
#include <queue>
#include <unordered_set>

#include <fs/core/state.hxx>
#include <fs/core/problem.hxx>
#include <fs/core/heuristics/relaxed_plan/rpg_data.hxx>
#include <fs/core/utils/utils.hxx>
#include <fs/core/utils/printers/printers.hxx>
#include <fs/core/utils/printers/actions.hxx>
#include <lapkt/tools/logging.hxx>
#include <fs/core/utils/config.hxx>
#include <fs/core/actions/action_id.hxx>

namespace fs0 {

class SupportedAction {
public:
	const ActionID* _action;
	std::shared_ptr<std::vector<Atom>> _support;
	
	SupportedAction(const ActionID* action, std::shared_ptr<std::vector<Atom>> support) : 
		_action(action), _support(support) {}
	
	inline bool operator==(const SupportedAction& rhs) const { 
		return _action == rhs._action && *_support == *(rhs._support);
	}
	
	inline bool operator!=(const SupportedAction& rhs) const {return !this->operator==(rhs);}
	inline bool operator< (const SupportedAction& rhs) const {
		return _action < rhs._action || (_action == rhs._action && *_support < *(rhs._support));
	}
	inline bool operator> (const SupportedAction& rhs) const {return  rhs.operator<(*this);}
	inline bool operator<=(const SupportedAction& rhs) const {return !this->operator>(rhs);}
	inline bool operator>=(const SupportedAction& rhs) const {return !this->operator<(rhs);}
	
	//! Prints a representation of the state to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const SupportedAction& o) { return o.print(os); }
	std::ostream& print(std::ostream& out) const {
		out << *_action << ", where: ";
		for (const auto& atom:*(_support))  out << atom << ", ";
		return out;
	}
};


/**
 * A Relaxed Plan extractor. This class is used to perform plan extraction from
 * an already existing RPG data structure. Two different subclasses exist differing
 * in the way in which the repeated application of the same actions is treated.
 */
template <typename RPGBookkeeping>
class BaseRelaxedPlanExtractor {
protected:
	const State& _seed;
	
	//! The book-keeping RPG data.
	const RPGBookkeeping& _data;
	
	std::unordered_set<Atom> processed;
	std::queue<Atom> pending;
	std::unordered_set<Atom> _relevant;

public:
	
	/**
	 * @param seed The original, non-relaxed state.
 	 * @param data The data structure representing the planning graph
	 */
	BaseRelaxedPlanExtractor(const State& seed, const RPGBookkeeping& data) :
		_seed(seed), _data(data), processed(), pending()
	{}
	
	virtual ~BaseRelaxedPlanExtractor() {}


	/**
	 * Builds the relaxed plan from the layered planning graph contained in the '_data' attribute,
	 * starting from the goal causes given in 'goalAtoms'.
	 * 
	 * @param goalAtoms The atoms that allowed the planning graph to reach a goal state.
	 */
	long computeRelaxedPlanCost(const std::vector<Atom>& goalAtoms) {
		enqueueAtoms(goalAtoms);
		
		while (!pending.empty()) {
			const Atom& atom = pending.front();
			processAtom(atom);
			pending.pop();
		}
		
		return buildRelaxedPlan();
	}
	
	const std::unordered_set<Atom>& get_relevant() const { return _relevant; }

protected:
	//! Put all the atoms in a given vector of atoms in the queue to be processed.
	inline void enqueueAtoms(const std::vector<Atom>& atoms) { for(auto& atom:atoms) pending.push(atom); }
	
	inline void mark_as_relevant(const std::vector<Atom>& atoms) { for(auto& atom:atoms) _relevant.insert(atom); }

	//! Process a single atom by seeking its supports left-to-right in the RPG and enqueuing them to be further processed
	void processAtom(const Atom& atom) {
		if (_seed.contains(atom)) return; // The atom was already on the seed state, thus has empty support.
		if (processed.find(atom) != processed.end()) return; // The atom has already been processed
		
		const typename RPGBookkeeping::AtomSupport& support = _data.getAtomSupport(atom);
		
		const ActionID* action_id = std::get<1>(support);
		_unused(action_id);
		assert(action_id);
		registerPlanAction(support);
		enqueueAtoms(*(std::get<2>(support))); // Push the full support of the atom
		mark_as_relevant(*(std::get<2>(support)));
		processed.insert(atom); // Tag the atom as processed.
	}
	
	virtual void registerPlanAction(const typename RPGBookkeeping::AtomSupport& support) = 0;
	
	virtual long buildRelaxedPlan() = 0;
};



/**
 * In a SupportedRelaxedPlanExtractor, a relaxed plan is composed not only of actions but
 * only of the conditions under which those actions are performed, e.g. an action being such as:
 * "move(right), when the current position is (1, 3)"
 * This yields longer relaxed plans.
 */
template <typename RPGBookkeeping>
class SupportedRelaxedPlanExtractor : public BaseRelaxedPlanExtractor<RPGBookkeeping> {
protected:
	std::set<SupportedAction> supporters;

public:
	/**
	 * @param seed The original, non-relaxed state.
 	 * @param data The data structure representing the planning graph
	 */
	SupportedRelaxedPlanExtractor(const State& seed, const RPGBookkeeping& data) :
		BaseRelaxedPlanExtractor<RPGBookkeeping>(seed, data), supporters()
	{}

protected:

	
	void registerPlanAction(const typename RPGBookkeeping::AtomSupport& support) {
		// Push the action along the full support of the particular atom
		supporters.insert(SupportedAction(std::get<1>(support), std::get<2>(support)));
	}
	
	long buildRelaxedPlan() {
		LPT_EDEBUG("relaxed-plan" , "Relaxed plan (" << supporters.size() << ") for state: " <<  std::endl << this->_seed << std::endl << "\t" << print::supported_plan(supporters) << std::endl);
		return (long) supporters.size();
	}
};

/**
 * In an PropositionalRelaxedPlanExtractor, a relaxed plan is simply composed of a list of actions,
 * not making any distinction with respect to the values of variables relevant for the action effects
 * under which the action is undertaken.
 */
template <typename RPGBookkeeping>
class PropositionalRelaxedPlanExtractor : public BaseRelaxedPlanExtractor<RPGBookkeeping> {
protected:
		std::vector<std::unordered_set<const ActionID*>> perLayerSupporters;

public:
	/**
	 * @param seed The original, non-relaxed state.
 	 * @param data The data structure representing the planning graph
	 */
	PropositionalRelaxedPlanExtractor(const State& seed, const RPGBookkeeping& data) :
		BaseRelaxedPlanExtractor<RPGBookkeeping>(seed, data), perLayerSupporters(data.getNumLayers())
	{}

protected:

	void registerPlanAction(const typename RPGBookkeeping::AtomSupport& support) {
		// We ignore the particular atom support and take only into account the action
		perLayerSupporters[std::get<0>(support)].insert(std::get<1>(support));
	}
	
	long buildRelaxedPlan() {
#ifndef DEBUG
		// In production mode, we simply count the number of actions in the plan, but prefer not to build the actual plan.
		unsigned size = 0;
		for (const auto& supporters:perLayerSupporters) {
			size += supporters.size();
		}
		return (long) size;
#endif

		// In debug mode, we build the relaxed plan by flattening the supporters at each layer, so that we can log the actual plan.
		plan_t plan;
		for (const auto& supporters:perLayerSupporters) {
			plan.insert(plan.end(), supporters.cbegin(), supporters.cend());
		}

		// Note that computing the relaxed heuristic by using some form of local consistency might yield plans that are not correct for the relaxation
		// assert(ActionManager::checkRelaxedPlanSuccessful(Problem::getInstance(), plan, _seed));
		LPT_EDEBUG("relaxed-plan" , "Relaxed plan (" << plan.size() << ") for state: " <<  std::endl << this->_seed << std::endl << "\t"  << print::plan(plan) << std::endl);

		return (long) plan.size();
	}
};

template <typename RPGBookkeeping>
class RelaxedPlanExtractorFactory {
public:
	static BaseRelaxedPlanExtractor<RPGBookkeeping>* create(const State& seed, const RPGBookkeeping& data) {
		const Config& config = Config::instance();
		if (config.getRPGExtractionType() == Config::RPGExtractionType::Propositional) {
			return new PropositionalRelaxedPlanExtractor<RPGBookkeeping>(seed, data);
		} else {
			return new SupportedRelaxedPlanExtractor<RPGBookkeeping>(seed, data);
		}
	}
};

} // namespaces
