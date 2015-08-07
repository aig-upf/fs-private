
#pragma once

#include <vector>
#include <ostream>
#include <problem_info.hxx>
#include <actions/ground_action.hxx>
#include <boost/units/detail/utility.hpp>

namespace fs0 {

class Problem;
class SupportedAction;

//! Helper class - debugging purposes only
class PlanPrinter {
protected:
	const ActionPlan& _plan;
public:
	PlanPrinter(const ActionPlan& plan) : _plan(plan) {}
	
	//! Prints a representation of the state to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const PlanPrinter& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	
	static void printSupportedPlan(const std::set<SupportedAction>& plan, std::ostream& out);
	
	//! static helpers
	static void printPlan(const ActionPlan& plan, const Problem& problem, std::ostream& out);
	static void printPlan(const std::vector<GroundAction::IdType>& plan, const ProblemInfo& problemInfo, std::ostream& out);
	static void printPlanJSON(const std::vector<GroundAction::IdType>& plan, const ProblemInfo& problemInfo, std::ostream& out);	
};

class SupportedPlanPrinter {
protected:
	const std::set<SupportedAction>& _plan;
public:
	SupportedPlanPrinter(const std::set<SupportedAction>& plan) : _plan(plan) {}
	
	//! Prints a representation of the state to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const SupportedPlanPrinter& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const {
		printSupportedPlan(_plan, os);
		return os;
	}
	
	static void printSupportedPlan(const std::set<SupportedAction>& plan, std::ostream& out);
};


namespace print {

//! Print the proper, demangled name of a std::type_info / std::type_index object
template <typename T>
std::string type_info_name(const T& type) { return boost::units::detail::demangle(type.name()); }

template <typename T>
class Vector {
	protected:
		const std::vector<T*>& _object;
	public:
		Vector(const std::vector<T*>& object) : _object(object) {}
		
		friend std::ostream& operator<<(std::ostream &os, const Vector& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};

//! A helper to allow for automatic template argument deduction
template <typename T>
const Vector<T> vector(const std::vector<T>& object) { return Vector<T>(object); }
}

} // namespaces
