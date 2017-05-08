#include <unordered_set>
#include <state.hxx>
#include <search/drivers/sbfws/hm_heuristic.hxx>
#include <utils/utils.hxx>
#include <languages/fstrips/language.hxx>
#include <languages/fstrips/scopes.hxx>

using namespace fs0;
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace drivers {
  
  
  //! Helper
VariableIdx derive_config_variable(ObjectIdx object_id) {
	const ProblemInfo& info = ProblemInfo::getInstance();
	std::string obj_name = info.deduceObjectName(object_id, info.getTypeId("object_id"));
	return info.getVariableId("confo(" + obj_name  +  ")");
}

//! Helper
VariableIdx derive_goal_config(ObjectIdx object_id, const fs::Formula* goal) {
	const ProblemInfo& info = ProblemInfo::getInstance();
	
	std::string obj_name = info.deduceObjectName(object_id, info.getTypeId("object_id"));
	VariableIdx obj_conf = info.getVariableId("confo(" + obj_name  +  ")");
	//Hack to get the goal confs
	VariableIdx goal_conf = info.getVariableId("goal_conf(" + obj_name  +  ")");

	
	
	// This only works for goal atoms of the form "confo(o1) = co5". It is non-symmetrical.
	/*for (auto atom:fs0::Utils::filter_by_type<const fs::EQAtomicFormula*>(goal->all_atoms())) {//this method does not exists!!!!!
		if (auto x = dynamic_cast<const fs::StateVariable*>(atom->lhs())) {
			if (x->getValue() == obj_conf) {
				auto y = dynamic_cast<const fs::Constant*>(atom->rhs());
				assert(y);
				ObjectIdx goal_conf = y->getValue();
				LPT_INFO("cout", "Goal config for object " << obj_name << ": " << info.deduceObjectName(goal_conf, info.getTypeId("conf_obj")));
				return goal_conf;
			}
		}
	}*/
	
	return -1;
}


void hMHeuristic::setup_goal_confs() {
      const ProblemInfo& info = ProblemInfo::getInstance();
	std::vector<ObjectIdx> goals(info.getNumObjects(), std::numeric_limits<int>::max());
	const UnaryFunction& target_f = info.get_extension<UnaryFunction>("goal_conf");
	for (ObjectIdx obj_id:info.getTypeObjects("object_id")) {
		std::cout << obj_id << " " << target_f.value(obj_id) << std::endl;
		_all_objects_goal.insert(std::make_pair(obj_id, target_f.value(obj_id)));
	}
}

VariableIdx derive_goal_config(ObjectIdx object_id, const std::vector<const fs::AtomicFormula*>& goal) {
	for (auto atom:goal) {
		VariableIdx var = derive_goal_config(object_id, atom);
		if (var != -1) return var;
	}
	return -1;
}




hMHeuristic::hMHeuristic(const fs::Formula* goal):
	_external(ProblemInfo::getInstance().get_external()),
	_idx_goal_atom(ProblemInfo::getInstance().getNumObjects(), -1)
{
	const ProblemInfo& info = ProblemInfo::getInstance();
	
	_holding_var = info.getVariableId("holding()");

	for (ObjectIdx obj_id:info.getTypeObjects("object_id")) {
		_all_objects_ids.push_back(obj_id);
		_all_objects_conf.push_back(derive_config_variable(obj_id));
		
		
		// If the object has a particular goal configuration, insert it.
		/*ObjectIdx goal_config = derive_goal_config(obj_id, goal);
		if (goal_config != -1) {
			_all_objects_goal.insert(std::make_pair(obj_id, goal_config));
		}*/
	}
	setup_goal_confs();

	/*Is it required?*/
	
	/*const fs::Conjunction* g = dynamic_cast<const fs::Conjunction*>(goal);
	assert(g);
	auto conjuncts = g->getConjuncts();

	for (ObjectIdx obj_id:info.getTypeObjects("object_id")) {
		std::string obj_name = info.deduceObjectName(obj_id, info.getTypeId("object_id"));
		VariableIdx co_var = info.getVariableId("confo(" + obj_name  +  ")");
		
		for (unsigned i = 0; i < conjuncts.size(); ++i) {
			const fs::AtomicFormula* condition = conjuncts[i];
			
			auto scope = fs::ScopeUtils::computeDirectScope(condition);
			if (scope.size() != 1) throw std::runtime_error("Unsupported goal type");
			
			if (co_var == scope[0]) {
				_idx_goal_atom[obj_id] = i;
				break;
			}
		}
	}*/
}


//! h(s)= number of goal objects that still need to be picked up and moved in s  * 2
//! -1; if goal object being held
unsigned hMHeuristic::evaluate(const State& s, const std::vector<bool>& is_path_to_goal_atom_clear) const {
	unsigned h = 0;
	
	ObjectIdx held_object = s.getValue(_holding_var); // The object which is currently being held
	
	for (unsigned i = 0; i < _all_objects_conf.size(); ++i) {
		ObjectIdx object_id = _all_objects_ids[i];
		VariableIdx object_conf = _all_objects_conf[i];
		
		auto it = _all_objects_goal.find(object_id);
		if (it == _all_objects_goal.end()) continue; // The object does not appear on the goal formula
		
		// otherwise, we can deduce the goal configuration of the object
		ObjectIdx goal_obj_conf =  it->second;
		
		ObjectIdx current_obj_conf = s.getValue(object_conf);
		if (current_obj_conf != goal_obj_conf) {
			
			if (held_object == object_id) {
				h = h+1;
			} else {
				h = h+2;
			}
		}
	}

	return h;
}




} } // namespaces