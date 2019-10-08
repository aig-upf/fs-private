
#pragma once

#include <string>
#include <unordered_map>
#include <boost/serialization/vector.hpp>

#include <fs/core/fs_types.hxx>

// Forward-declare the basic SddNode, Vtree and SDDManager typedefs from the SDD API
struct sdd_node_t; typedef struct sdd_node_t SddNode;
struct vtree_t; typedef struct vtree_t Vtree;
struct sdd_manager_t; typedef struct sdd_manager_t SddManager;

namespace fs0 {

class State;
class PartiallyGroundedAction;

class ActionSchemaSDD {
public:
    ActionSchemaSDD(unsigned schema_id_, std::vector<std::pair<VariableIdx, unsigned>> relevant, SddManager* manager, Vtree* vtree, SddNode* sddnode);
    ~ActionSchemaSDD();

    SddNode* conjoin_with(const State& state) const;
    unsigned var_count() const;

    unsigned schema_id() const { return schema_id_; }

protected:
    unsigned schema_id_;

    //! The actual SDD objects
    SddManager* sddmanager_;
    Vtree* vtree_;
    SddNode* sddnode_;

    //! Some bookkeeping data

    //! A list of the relevant atoms along with their SDD id. For instance, a tuple
    //! <v, id> in vector `relevant_` means that atom v=true is relevant to the SDD
    //! for this action schema, and its ID within the SDD is `id`. In a STRIPS problem, we might
    //! typically have a triple like <holding(a), 4>
    std::vector<std::pair<VariableIdx, unsigned>> relevant_;
    
    std::vector<unsigned> bindings_; // TODO Determine best data structure
};

class SDDModel {
    enum class value_t {Undefined, False, True};

public:
    explicit SDDModel(unsigned nvars) : values_(nvars, value_t::Undefined) {}

protected:
    std::vector<value_t> values_;
};

class SDDModelIterator {
public:
    SDDModelIterator() : _index(0) { throw std::runtime_error("TO BE IMPLEMENTED"); }
    SDDModel* next() { assert(is_valid()); throw std::runtime_error("TO BE IMPLEMENTED"); }

    bool is_valid() const { return true; }

protected:
    std::size_t _index;

};

//! Loads from disk all SDDs in the given directory (one per action schema)
std::vector<std::shared_ptr<ActionSchemaSDD>> load_sdds_from_disk(const std::vector<const PartiallyGroundedAction*>& schemas, const std::string& dir);

} // namespaces