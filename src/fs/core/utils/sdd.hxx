
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

    SddManager* manager() { return sddmanager_; }

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

//! A (possibly partial) SDD model, that is, a mapping between the atoms in the SDD and a truth
//! value that satisfies the SDD. For efficiency reasons, we allow for undefined values, so that
//! properly speaking we can have assignments that satisfy some subtree of the original SDD we are interested in
class SDDModel {
public:
    enum class value_t : uint8_t {Undefined, False, True};

    explicit SDDModel(unsigned nvars) : values_(nvars, value_t::Undefined) {}
    explicit SDDModel(std::vector<value_t>&& values) : values_(std::move(values)) {}

    //! Default copy constructors and assignment operators
    SDDModel(const SDDModel&) = default;
    SDDModel(SDDModel&&) = default;
    SDDModel& operator=(const SDDModel&) = default;
    SDDModel& operator=(SDDModel&&) = default;

    bool operator==(const SDDModel &rhs) const { return values_ == rhs.values_; }
    bool operator!=(const SDDModel &rhs) const { return !(this->operator==(rhs));}

    inline const value_t& operator[] (std::size_t i) const { assert(i < values_.size()); return values_[i]; }
    inline value_t& operator[] (std::size_t i) { assert(i < values_.size()); return values_[i]; }
    inline std::size_t size() const { return values_.size(); }

    static SDDModel merge_disjoint_models(const SDDModel& left, const SDDModel& right);

protected:
    std::vector<value_t> values_;
};


class SDDModelEnumerator {
public:
    explicit SDDModelEnumerator(SddManager* manager);

    std::vector<SDDModel> models(SddNode* node, Vtree* vtree= nullptr);

protected:
    SddManager* sddmanager_;
    unsigned nvars_;

    std::vector<SDDModel> model_cross_product(SddNode* leftnode, SddNode* rightnode, Vtree* leftvt, Vtree* rightvt);
    void model_cross_product(SddNode* leftnode, SddNode* rightnode, Vtree* leftvt, Vtree* rightvt, std::vector<SDDModel>& output);
};

//! Loads from disk all SDDs in the given directory (one per action schema)
std::vector<std::shared_ptr<ActionSchemaSDD>> load_sdds_from_disk(const std::vector<const PartiallyGroundedAction*>& schemas, const std::string& dir);

} // namespaces