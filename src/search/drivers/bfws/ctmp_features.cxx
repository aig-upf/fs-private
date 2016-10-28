
#include <unordered_set>

#include <aptk2/tools/logging.hxx>

#include <state.hxx>
#include <search/drivers/bfws/ctmp_features.hxx>
#include <utils/utils.hxx>
#include <utils/printers/feature_set.hxx>
#include <languages/fstrips/language.hxx>

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
	
	// This only works for goal atoms of the form "confo(o1) = co5". It is non-symmetrical.
	for (auto atom:fs0::Utils::filter_by_type<const fs::EQAtomicFormula*>(goal->all_atoms())) {
		if (auto x = dynamic_cast<const fs::StateVariable*>(atom->lhs())) {
			if (x->getValue() == obj_conf) {
				auto y = dynamic_cast<const fs::Constant*>(atom->rhs());
				assert(y);
				ObjectIdx goal_conf = y->getValue();
				LPT_INFO("cout", "Goal config for object " << obj_name << ": " << info.deduceObjectName(goal_conf, info.getTypeId("conf_obj")));
				return goal_conf;
			}
		}
	}
	
	return -1;
}



PlaceableFeature::PlaceableFeature(bool check_final_overlaps, const fs::Formula* goal)
	:
	_external(ProblemInfo::getInstance().get_external()),
	_check_overlaps(check_final_overlaps)
{
	const ProblemInfo& info = ProblemInfo::getInstance();
	
	_no_object_id = info.getObjectId("no_object");
	_holding_var = info.getVariableId("holding()");
	_confb_rob = info.getVariableId("confb(rob)");
	_confa_rob = info.getVariableId("confa(rob)");
	_goal_obj_conf = info.getObjectId("co5");
	

	for (ObjectIdx obj_id:info.getTypeObjects("object_id")) {
		_all_objects_ids.push_back(obj_id);
		_all_objects_conf.push_back(derive_config_variable(obj_id));
		
		// If the object has a particular goal configuration, insert it.
		ObjectIdx goal_config = derive_goal_config(obj_id, goal);
		if (goal_config != -1) {
			_all_objects_goal.insert(std::make_pair(obj_id, goal_config));
		}
	}
}

aptk::ValueIndex
PlaceableFeature::evaluate(const State& s) const {
	
	// If no object is being held, return a NULL object ID
	ObjectIdx held_object = s.getValue(_holding_var);
	if (held_object == _no_object_id) {
		return 0;
	}
	
	// Otherwise, ensure that the robot is in a "placeable" overall configuration
	auto rob_conf = {s.getValue(_confb_rob), s.getValue(_confa_rob)};
	bool placeable = _external.placeable(rob_conf);
	if (!placeable) return 0;
	
	// And ensure that the resulting configuration would be a goal configuration for the object being placed
	ObjectIdx future_object_conf = _external.placing_pose(rob_conf);
	auto it = _all_objects_goal.find(held_object);
	
	if (it == _all_objects_goal.end() ||
		(it != _all_objects_goal.end() && future_object_conf != it->second)) {
		// i.e. iff no goal config  was specified for the currently-held object, 
		// or it was but it is not the future config where it would be placed, return false.
		return 0;
	}
	
	// And, eventually, that there is no overlap.
	if (_check_overlaps) {
		// The configuration on which the object will remain once I place it
		// does not overlap with the configuration of any other object.
		
		assert(_all_objects_conf.size()==_all_objects_ids.size());
		for (unsigned i = 0; i < _all_objects_conf.size(); ++i) {
			ObjectIdx other_object_id = _all_objects_ids[i];
			if (other_object_id != held_object) {
				VariableIdx other_object_var = _all_objects_conf[i];
				ObjectIdx other_obj_conf = s.getValue(other_object_var);
				bool objects_overlap = _external.nonoverlap_oo({future_object_conf, other_obj_conf});
				if (objects_overlap) return 0;
			}
		}
	}
	
	// Otherwise, the object being held is placeable
	return held_object;
}


	
GraspableFeature::GraspableFeature()
	:
	_external(ProblemInfo::getInstance().get_external())
{
	const ProblemInfo& info = ProblemInfo::getInstance();
	
	_confb_rob = info.getVariableId("confb(rob)");
	_confa_rob = info.getVariableId("confa(rob)");
	_holding_var = info.getVariableId("holding()");
	_no_object_id = info.getObjectId("no_object");

	TypeIdx obj_t = info.getTypeId("object_id");
	for (ObjectIdx other_obj_id:info.getTypeObjects(obj_t)) {
		_all_objects_conf.push_back(derive_config_variable(other_obj_id));
	}
}


aptk::ValueIndex
GraspableFeature::evaluate(const State& s) const {
	ObjectIdx held_object = s.getValue(_holding_var);
	if (held_object != _no_object_id) return 0; // Some object is already held.
	
	auto cb = s.getValue(_confb_rob);
	auto ca = s.getValue(_confa_rob);
	
	// Return an identifier of the first object that is graspable, or 0, if none
	for (VariableIdx other_object_var:_all_objects_conf) {
		ObjectIdx co = s.getValue(other_object_var);
		bool graspable = _external.graspable({cb, ca, co});
		if (graspable) return other_object_var;
	}

	return 0;
}

GlobalRobotConfFeature::GlobalRobotConfFeature() {
	const ProblemInfo& info = ProblemInfo::getInstance();
	_confb_rob = info.getVariableId("confb(rob)");
	_confa_rob = info.getVariableId("confa(rob)");
}


aptk::ValueIndex
GlobalRobotConfFeature::evaluate(const State& s) const {
	return s.getValue(_confa_rob) * 1000 + s.getValue(_confb_rob);
}


} } // namespaces


