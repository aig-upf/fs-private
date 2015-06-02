
#pragma once

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <unordered_set>
#include <unordered_map>

#include <fs0_types.hxx>
#include <problem_info.hxx>
#include <utils/serializer.hxx>
#include <constraints/constraint_factory.hxx>

using namespace fs0;

/*********************************************/
/* The static data                           */
/*********************************************/
class ExternalBase  {
protected:
    const ProblemInfo& _problemInfo;

    

public:
    ExternalBase(const ProblemInfo& problemInfo, const std::string& data_dir) : _problemInfo(problemInfo) 
    {}

    static ScopedConstraint::cptr instantiateConstraint(const std::string& classname, const ObjectIdxVector& parameters, const VariableIdxVector& variables) {
		return ConstraintFactory::create(classname, parameters, variables); // By default we try to instantiate the constraint as a global constraint
    }

    
};

