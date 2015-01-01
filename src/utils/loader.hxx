
#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <functional>

#include <core_types.hxx>
#include <state.hxx>
#include <actions.hxx>

namespace aptk { namespace core {
	class Problem;
} }

namespace aptk { namespace core { namespace utils {

class StateLoader
{
public:
	/**
	 * Loads a state specification for a given text file.
	 * The specification basically consists on an assignation of values to all the state variables.
	 */
	static const State::cptr loadStateFromFile(const std::string& filename);
	
	/**
	 * Instantiates all the grounded actions from the information stored in the given text files and by calling
	 * the specified function, which is generated dinamically and performs the actual instantiation.
	 */
	static void loadGroundedActions(
		const std::string& filename, 
		std::function<CoreAction::cptr (
			const std::string&,
			const ObjectIdxVector&,
			const ObjectIdxVector&,
			const std::vector<VariableIdxVector>&,
			const std::vector<VariableIdxVector>&,
			const std::vector<VariableIdxVector>&
			)> actionFactory,
		Problem& problem);
	
	static void generateGoalConstraints(
		const std::string& filename, 
		std::function<ApplicableEntity::cptr (const std::vector<VariableIdxVector>&)> goalFactory,
		Problem& problem);
	
	/**
	 * Loads a set of state constraints.
	 */
	static void loadConstraints(const std::string& filename, Problem& problem, bool goal);
	
protected:
	template<typename T>
	static std::vector<T> parseNumberList(const std::string& input, const std::string sep = ",");
	
	template<typename T>
	static std::vector<std::vector<T>> parseDoubleNumberList(const std::string& input, const std::string sep = "/");
	
	/*
	//! We currently only parse Plain Conjunctions, i.e. conjunctions of either facts or negated facts.
	static PlainConjunctiveFact::cptr parseFactList(const std::string& input, const std::string sep = ",") {
		std::vector<Fact::cptr> facts;
		std::vector<std::string> strs;
		boost::split(strs, input, boost::is_any_of(sep));
		for(const auto& str:strs) {
			// 'str' is of the form "x=y", or "x!y" (meaning x!=y)
			std::vector<std::string> elements;
			boost::split(elements, str, boost::is_any_of("!="));
			assert(elements.size()==2);
			if (str.find('=') != std::string::npos) {
				facts.push_back(std::make_shared<Fact>(std::stoi(elements[0]), std::stoi(elements[1])));
			}
			else if (str.find('!') != std::string::npos) {
				facts.push_back(std::make_shared<NegatedFact>(std::stoi(elements[0]), std::stoi(elements[1])));
			}
			else assert(false);
		}
		return std::make_shared<const PlainConjunctiveFact>(facts);
	}
	*/
};

} } } // namespaces

