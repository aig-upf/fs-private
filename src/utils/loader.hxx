
#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <state.hxx>
#include <core_problem.hxx>
#include <constraints/factory.hxx>


namespace aptk { namespace core { namespace utils {


class StateLoader
{
public:
	/**
	 * Loads a state specification for a given text file.
	 * The specification basically consists on an assignation of values to all the state variables.
	 */
	static const State::cptr loadStateFromFile(const std::string& filename) {
		FactVector facts;
		std::string str;
		std::ifstream in(filename);
		
		// Parse the total number of facts
		std::getline(in, str);
		unsigned numFacts = std::stoi(str);
		
		// Parse the initial facts themselves
		std::getline(in, str);
		typedef boost::tokenizer<boost::escaped_list_separator<char> > Tokenizer;
		Tokenizer tok(str);
		for (auto factStr:tok) {
			std::vector<std::string> strs;
			boost::split(strs, factStr, boost::is_any_of("="));
			facts.push_back(Fact(std::stoi(strs[0]), std::stoi(strs[1])));
		}
// 		std::cout << numFacts << std::endl;
// 		for (auto f: facts) std::cout << f << std::endl;
		return std::make_shared<State>(numFacts, facts);
	}
	
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
		Problem& problem)
	{
		std::string line;
		std::ifstream in(filename);
		
		// Parse the ground actions - each line is an action
		while (std::getline(in, line)) {
			// std::cout << line << std::endl;
			std::vector<std::string> strs;
			boost::split(strs, line, boost::is_any_of("#"));
			
			// strs contains 7 elements: 
			// # (0) Action name
            // # (1) classname
            // # (2) binding
            // # (3) derived objects
            // # (4) applicability relevant vars
            // # (5) effect relevant vars
            // # (6) effect affected vars
			assert(strs.size()==7);
			
			
			// We ignore the grounded name for the moment being.
			const std::string& actionClassname = strs[1];
			ObjectIdxVector binding = parseNumberList<int>(strs[2]);
			ObjectIdxVector derived = parseNumberList<int>(strs[3]);
			std::vector<VariableIdxVector> appRelevantVars = parseDoubleNumberList<unsigned>(strs[4]);
			std::vector<VariableIdxVector> effRelevantVars = parseDoubleNumberList<unsigned>(strs[5]);
			std::vector<VariableIdxVector> effAffectedVars = parseDoubleNumberList<unsigned>(strs[6]);

			auto aptr = actionFactory(actionClassname, binding, derived, appRelevantVars, effRelevantVars, effAffectedVars);
			problem.addAction(aptr);
		}
	}
	
	/**
	 * Loads a goal especification from a given text file.
	 */
	static void loadGoalEvaluator(
		const std::string& filename, 
		std::function<ApplicableEntity::cptr (const std::vector<VariableIdxVector>&)> goalFactory,
		Problem& problem)
	{
		std::string line;
		std::ifstream in(filename);
		
		// Parse the goal relevant variables - we simply have one line with the variables indexes.
		std::getline(in, line);
		
		std::vector<VariableIdxVector> appRelevantVars = parseDoubleNumberList<unsigned>(line);
		problem.setGoalEvaluator(goalFactory(appRelevantVars));
	}
	
	/**
	 * Loads a set of state constraints.
	 */
	static void loadConstraints(const std::string& filename, Problem& problem, bool goal) {
		std::string line;
		std::ifstream in(filename);
		
		// Each line encodes a constraint.
		while (std::getline(in, line)) {
			std::vector<std::string> strs;
			boost::split(strs, line, boost::is_any_of("#"));
			
			// strs contains 3 elements: 
			// (0) The constraint description
            // (1) The constraint name
            // (2) The variables upon which the constraint is enforced
			assert(strs.size()==3);

			// We ignore the grounded name for the moment being.
			const std::string& name = strs[1];
			VariableIdxVector variables = parseNumberList<unsigned>(strs[2]);
			
			if (goal) {
				problem.registerGoalConstraint(ConstraintFactory::create(name, variables));
			} else {
				problem.registerConstraint(ConstraintFactory::create(name, variables));
			}
		}
	}
	
protected:
	template<typename T>
	static std::vector<T> parseNumberList(const std::string& input, const std::string sep = ",") {
		if (input == "") return std::vector<T>();
		
		std::vector<T> output;
		
		std::vector<std::string> strs;
		boost::split(strs, input, boost::is_any_of(sep));

		for(const auto& str:strs) {
			output.push_back(boost::lexical_cast<T>(str));
		}
		return output;
	}
	
	template<typename T>
	static std::vector<std::vector<T>> parseDoubleNumberList(const std::string& input, const std::string sep = "/") {
		std::vector<std::vector<T>> output;
		std::vector<std::string> strs;
		boost::split(strs, input, boost::is_any_of(sep));
		
		if (strs.size() == 0) {
			output.push_back(std::vector<T>());
		} else {
			for(const auto& str:strs) {
				output.push_back(parseNumberList<T>(str));
			}
		}
		return output;
	}
	
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
};

} } } // namespaces

