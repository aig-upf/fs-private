
#pragma once

#include <utility>
#include <memory>
#include <vector>
#include <unordered_map>

#include <fs/core/fs_types.hxx>
#include <fs/core/fstrips/fstrips_fwd.hxx>


namespace fs0::fstrips {

class Problem;
class GroundAction;

class Grounding {
public:
    Grounding(std::shared_ptr<LanguageInfo> language, const Problem& problem) :
        language_(std::move(language)), problem_(problem)
    {}

    VariableIdx add_state_variable(const symbol_id& symbol, const std::vector<object_id>& point);

    unsigned add_schema_grounding(unsigned schema_id, const std::vector<object_id>& point);

    std::string compute_state_variable_name(const symbol_id& symbol, const std::vector<object_id>& point);

    //! Print a representation of the object to the given stream.
    friend std::ostream& operator<<(std::ostream &os, const Grounding& o) { return o.print(os); }
    std::ostream& print(std::ostream& os) const;

protected:
    std::shared_ptr<LanguageInfo> language_;

    const Problem& problem_;

    /// Ground state variables ///
    //! A map from state variable ID to state variable name
    std::vector<std::string> variableNames;

    //! A map from state variable name to state variable ID
    std::unordered_map<std::string, VariableIdx> variableIds;

    //! A map from the actual data "f(t1, t2, ..., tn)" to the assigned variable ID
    std::map<std::pair<unsigned, std::vector<object_id>>, VariableIdx> variableDataToId;
    std::vector<std::pair<unsigned, std::vector<object_id>>> variableIdToData;

    //! Mapping from state variable index to the type associated to the state variable
    std::vector<type_id> _sv_types;

    //! Maps variable index to type index
    std::vector<TypeIdx> variableTypes;

    /// Ground actions ///
    std::vector<const GroundAction*> ground_actions_;
};

} // namespaces
