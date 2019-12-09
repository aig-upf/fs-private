
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
class ActionData;
class PropositionalSchematicAction;


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

class ActionSchemaSDD {
public:
    ActionSchemaSDD(const std::shared_ptr<PropositionalSchematicAction>& schema,
            std::vector<std::pair<VariableIdx, unsigned>> relevant,
            std::vector<std::vector<std::pair<object_id, unsigned>>> bindings,
            SddManager* manager, Vtree* vtree, SddNode* sddnode);
    ~ActionSchemaSDD();

    SddNode* conjoin_with(const State& state) const;
    unsigned var_count() const;

    SDDModel collect_state_literals(const State &state) const;

    SddManager* manager() { return sddmanager_; }

    SddNode* node() { return sddnode_; }

    const PropositionalSchematicAction& schematic_action() const {
        return *schema_;
    }

    std::vector<object_id> get_binding_from_model(const SDDModel& model);

    void report_sdd_stats() const;

    void collect_sdd_garbage(SddNode* node = nullptr) const;

    static size_t minimize_sdd(SddManager* manager, SddNode* node, unsigned int time_limit);


protected:
    const std::shared_ptr<PropositionalSchematicAction>& schema_;

    //! The actual SDD objects
    SddManager* sddmanager_;
    Vtree* vtree_;
    SddNode* sddnode_;

    //! Some bookkeeping data

    //! A list of the relevant atoms along with their SDD id. For instance, a tuple
    //! <v, id> in vector `relevant_` means that atom v=true is relevant to the SDD
    //! for this action schema, and its ID within the SDD is `id`. In a STRIPS problem, we might
    //! typically have a pair like <holding(a), 4>
    std::vector<std::pair<VariableIdx, unsigned>> relevant_;

    //! 'bindings_[i]' contains the object_id corresponding
    std::vector<std::vector<std::pair<object_id, unsigned>>> bindings_;
};


class RecursiveModelEnumerator {
public:
    using index_t = unsigned;
    using resultset_t = std::vector<index_t>;

    RecursiveModelEnumerator(SddManager* manager, SDDModel&& fixed);
    virtual ~RecursiveModelEnumerator() = default;

    std::vector<SDDModel> models(SddNode* node);
    resultset_t models(SddNode* node, Vtree* vtree);


protected:
    using node_id_t = std::pair<std::size_t, Vtree*>;

    SddManager* sddmanager_;
    unsigned nvars_;
    const SDDModel fixed_;
    unsigned long cache_hits_;
    unsigned long computed_nodes_;
    std::vector<SDDModel> model_register_;

    std::unordered_set<node_id_t, boost::hash<node_id_t>> computed_;

    //! A map between node id and resulting models
    std::unordered_map<node_id_t, resultset_t, boost::hash<node_id_t>> cache_;

    const resultset_t& models_with_cache(SddNode* node, Vtree* vtree);

    //    index_t register_model();
    index_t register_model(unsigned var, const SDDModel::value_t& val);
    index_t register_model(SDDModel&& model);

    resultset_t model_cross_product(SddNode* leftnode, SddNode* rightnode, Vtree* leftvt, Vtree* rightvt);
    void model_cross_product(SddNode* leftnode, SddNode* rightnode, Vtree* leftvt, Vtree* rightvt, std::vector<index_t>& output);


    bool node_is_false_in_fixed(SddNode* node);
};

class DFSModelEnumerator {
public:
    DFSModelEnumerator(SddManager* manager, SddNode* root, SDDModel&& fixed);

    std::vector<SDDModel> models(SddNode* node, Vtree* vtree=nullptr);

    SDDModel next();

    unsigned nvars() const { return nvars_; }

protected:
    SddManager* sddmanager_;
    SddNode* root_;
    unsigned nvars_;
    const SDDModel fixed_;

    bool node_is_false_in_fixed(SddNode* node);

    void next_(SddNode* node, Vtree* vtree, std::vector<SDDModel::value_t>& current);


    enum class case_t {
        case0
    };

    using decision_t = std::tuple<case_t, std::vector<SddNode*>, Vtree*>;
    using state_t = std::vector<decision_t>;
    state_t state_;
};


//! Loads from disk all SDDs in the given directory (one per action schema)
std::vector<std::shared_ptr<ActionSchemaSDD>> load_sdds_from_disk(const std::vector<const ActionData*>& schemas, const std::string& dir);

} // namespaces