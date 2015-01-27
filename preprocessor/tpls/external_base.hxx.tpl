
#pragma once

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <unordered_set>
#include <unordered_map>

#include <fs0_types.hxx>
#include <utils/serializer.hxx>
#include <constraints/constraint_factory.hxx>

using namespace fs0;

/*********************************************/
/* The static data                           */
/*********************************************/
class ExternalBase  {
protected:
    ${data_declarations}

public:
    ExternalBase(const std::string& data_dir) ${data_initialization}
    {}

    static ScopedConstraint::cptr instantiateConstraint(const std::string& classname, const ObjectIdxVector& parameters, const VariableIdxVector& variables) {
		return ConstraintFactory::create(classname, parameters, variables); // By default we try to instantiate the constraint as a global constraint
    }

    ${data_accessors}
};

