
#pragma once

#include <vector>
#include <queue>
#include <unordered_set>

#include <fs_types.hxx>


namespace fs0 { class State; class ActionID; class AtomIndex; class Atom; }
namespace fs0 { namespace gecode {

class RPGIndex;

/**
 * In an LiftedPlanExtractor, a relaxed plan is simply composed of a list of actions,
 * not making any distinction with respect to the values of variables relevant for the action effects
 * under which the action is undertaken.
 */
class LiftedPlanExtractor {
protected:
	const RPGIndex& _graph;
	
	std::set<AtomIdx> processed;
	std::queue<AtomIdx> pending;
	
	std::vector<std::unordered_set<const ActionID*>> perLayerSupporters;
	
	const AtomIndex& _tuple_index;

public:
	
	LiftedPlanExtractor(const RPGIndex& graph, const AtomIndex& tuple_index);
	virtual ~LiftedPlanExtractor() = default;


	/**
	 * Builds the relaxed plan from the layered planning graph contained in the '_graph' attribute,
	 * starting from the goal causes given in 'goalAtoms'.
	 * 
	 * @param goalAtoms The atoms that allowed the planning graph to reach a goal state.
	 */
	long computeRelaxedPlanCost(const std::vector<AtomIdx>& goal_support, std::vector<Atom>& relevant);
	
protected:
	//! Put all the atoms in a given vector of atoms in the queue to be processed.
	void enqueueTuples(const std::vector<AtomIdx>& tuples);

	//! Process a single atom by seeking its supports left-to-right in the RPG and enqueuing them to be further processed
	void processTuple(AtomIdx tuple, std::vector<Atom>& relevant);
	
	
	long buildRelaxedPlan();
};

} } // namespaces
