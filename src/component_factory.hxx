
#pragma once

#include <fs0_types.hxx>
#include <languages/fstrips/schemata.hxx>
#include <constraints/direct/constraint.hxx>

namespace fs = fs0::language::fstrips;

namespace fs0 {

//! An interface for the concrete instance generators
class BaseComponentFactory {
public:
//     virtual Action::cptr instantiateAction(
//         const std::string& classname,
// 		const ObjectIdxVector& binding,
// 		const ObjectIdxVector& derived,
//         const std::vector<VariableIdxVector>& appRelevantVars,
//         const std::vector<VariableIdxVector>& effRelevantVars,
//         const VariableIdxVector& effAffectedVars
//     ) const = 0;

//     virtual fs::AtomicFormulaSchema::cptr instantiateGoal(const std::vector<VariableIdxVector>& appRelevantVars) const = 0;

//     virtual DirectConstraint::cptr instantiateConstraint(const std::string& classname, const ObjectIdxVector& parameters, const VariableIdxVector& variables) const = 0;
    
    virtual std::map<std::string, Function> instantiateFunctions() const = 0;
};

} // namespaces