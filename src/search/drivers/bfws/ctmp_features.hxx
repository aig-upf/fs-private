
#pragma once

#include <memory>

#include <heuristics/novelty/features.hxx>
#include <unordered_map>
#include <utils/external.hxx>


namespace fs0 { class Config; }

namespace fs0 { namespace language { namespace fstrips { class Formula; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace drivers {

//! Helper
VariableIdx derive_goal_config(ObjectIdx object_id, const fs::Formula* goal);
VariableIdx derive_goal_config(ObjectIdx object_id, const std::vector<const fs::AtomicFormula*>& goal);

//! A feature representing the value of any arbitrary language term, e.g. X+Y, or @proc(Y,Z)
class PlaceableFeature : public NoveltyFeature {
public:
	PlaceableFeature(bool check_final_overlaps, const fs::Formula* goal);
	~PlaceableFeature() = default;
	PlaceableFeature(const PlaceableFeature&) = default;
	
	NoveltyFeature* clone() const override { return new PlaceableFeature(*this); }
	
	aptk::ValueIndex evaluate(const State& s) const override;
	
	std::ostream& print(std::ostream& os) const override {
		return os << "placeable*(check_overlaps=" << _check_overlaps << ")";
	}

protected:
	ObjectIdx _no_object_id;
	VariableIdx _holding_var;
	
	// The two following vectors are sync'd, i.e. _all_objects_conf[i] is the config of object _all_objects_ids[i]
	std::vector<ObjectIdx> _all_objects_ids; // The Ids of all objects
	std::vector<VariableIdx> _all_objects_conf; // The state variables of the configurations of all objects
	std::unordered_map<ObjectIdx, ObjectIdx> _all_objects_goal; // The configuration in the goal of each object, if any
	
	VariableIdx _confb_rob;
	VariableIdx _confa_rob;
	const ExternalI& _external;
	bool _check_overlaps;
};


//! A feature representing the object which is graspable, if any, given the current configuration of the robot
class GraspableFeature : public NoveltyFeature {
public:
	
	GraspableFeature();
	~GraspableFeature() = default;
	GraspableFeature(const GraspableFeature&) = default;
	NoveltyFeature* clone() const override { return new GraspableFeature(*this); }
	
	aptk::ValueIndex evaluate(const State& s) const override;
	
	std::ostream& print(std::ostream& os) const override {
		return os << "graspable()";
	}

protected:
	std::vector<VariableIdx> _all_objects_conf; // The state variables of the configurations of allobjects
	
	ObjectIdx _no_object_id;
	VariableIdx _holding_var;
	VariableIdx _confb_rob;
	VariableIdx _confa_rob;
	const ExternalI& _external;
};

//! A feature representing the value of any arbitrary language term, e.g. X+Y, or @proc(Y,Z)
class GlobalRobotConfFeature : public NoveltyFeature {
public:
	GlobalRobotConfFeature();
	~GlobalRobotConfFeature() = default;
	GlobalRobotConfFeature(const GlobalRobotConfFeature&) = default;
	NoveltyFeature* clone() const override { return new GlobalRobotConfFeature(*this); }
	
	aptk::ValueIndex evaluate(const State& s) const override;
	
	std::ostream& print(std::ostream& os) const override {
		return os << "global_rob_conf";
	}

protected:
	VariableIdx _confb_rob;
	VariableIdx _confa_rob;
};

//! A custom heuristic for the CTMP problem
//! h(s)= number of goal objects that still need to be picked up and moved in s  * 2
//! +  1; if goal object being held
class CustomHeuristic {
public:
	CustomHeuristic(const fs::Formula* goal);
	~CustomHeuristic() = default;
	CustomHeuristic(const CustomHeuristic&) = default;
	

	long evaluate(const State& s) const;

protected:
	// The two following vectors are sync'd, i.e. _all_objects_conf[i] is the config of object _all_objects_ids[i]
	std::vector<ObjectIdx> _all_objects_ids; // The Ids of all objects
	std::vector<VariableIdx> _all_objects_conf; // The state variables of the configurations of all objects
	std::unordered_map<ObjectIdx, ObjectIdx> _all_objects_goal; // The configuration in the goal of each object, if any
	
	VariableIdx _holding_var;
	const ExternalI& _external;
};

} } // namespaces



