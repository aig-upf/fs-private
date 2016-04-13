
#pragma once

#include <vector>
#include <queue>
#include <unordered_set>

#include <fs_types.hxx>


namespace fs0 { class State; class ActionID; class TupleIndex; }
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
	
	std::set<TupleIdx> processed;
	std::queue<TupleIdx> pending;
	
	std::vector<std::unordered_set<const ActionID*>> perLayerSupporters;
	
	const TupleIndex& _tuple_index;

public:
	
	LiftedPlanExtractor(const RPGIndex& graph, const TupleIndex& tuple_index);
	virtual ~LiftedPlanExtractor() = default;


	/**
	 * Builds the relaxed plan from the layered planning graph contained in the '_graph' attribute,
	 * starting from the goal causes given in 'goalAtoms'.
	 * 
	 * @param goalAtoms The atoms that allowed the planning graph to reach a goal state.
	 */
	long computeRelaxedPlanCost(const std::vector<TupleIdx>& tuples);
	
protected:
	//! Put all the atoms in a given vector of atoms in the queue to be processed.
	inline void enqueueTuples(const std::vector<TupleIdx>& tuples) { for(const auto& tuple:tuples) pending.push(tuple); }

	//! Process a single atom by seeking its supports left-to-right in the RPG and enqueuing them to be further processed
	void processTuple(TupleIdx tuple);
	
	
	long buildRelaxedPlan();
};

} } // namespaces
