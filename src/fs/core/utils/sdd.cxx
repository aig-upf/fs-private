
#include <fs/core/problem_info.hxx>
#include <fs/core/actions/actions.hxx>
#include <fs/core/utils/sdd.hxx>
#include <fs/core/utils/lexical_cast.hxx>
#include <fs/core/state.hxx>
#include <fs/core/utils/system.hxx>
#include <lapkt/tools/logging.hxx>

#include <lapkt/tools/resources_control.hxx>



#include <sdd/sddapi.hxx>

#include <boost/filesystem.hpp>   // includes all needed Boost.Filesystem declarations
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include <iostream>
#include <memory>
#include <fs/core/utils/config.hxx>

namespace fsys = boost::filesystem;
using boost::format;

namespace fs0 {

unsigned varid(SddLiteral literal) {
    if (literal == 0) throw std::runtime_error("Given literal has value 0");
    return literal < 0 ? -1 * literal : literal;  // i.e. the absolute value
}

SDDModel::value_t truth_value(SddNode* node) {
    assert (sdd_node_is_literal(node));
    return sdd_node_literal(node) < 0 ? SDDModel::value_t::False : SDDModel::value_t::True;
}

//! Loads from disk all SDDs in the given directory (one per action schema)
std::vector<std::shared_ptr<ActionSchemaSDD>>
load_sdds_from_disk(const std::vector<const PartiallyGroundedAction*>& schemas, const std::string& dir) {
    const ProblemInfo& info = ProblemInfo::getInstance();
    const Config& config = Config::instance();

    std::vector<std::shared_ptr<ActionSchemaSDD>> sdds;

    fsys::path path(dir);
    if (!fsys::exists(path)) throw std::runtime_error("Non-existing base SDD directory: " + dir);

    for (const auto& schema:schemas) {
        // Each action schema has a number of filenames starting with the name of the schema
        const std::string& schema_name = schema->getName();
        std::string mng_fname = str(format("%1%.manager.sdd") % schema_name);
        std::string vtree_fname = str(format("%1%.vtree.sdd") % schema_name);
        std::string atoms_fname = str(format("%1%.atoms.data") % schema_name);
        std::string bindings_fname = str(format("%1%.bindings.data") % schema_name);

        // Load vtree and manager
        LPT_DEBUG("cout", "Loading SDD and bookkeeping info corresponding to action \"" << schema_name << "\"");
        LPT_DEBUG("cout", "Mem. usage: " << get_current_memory_in_kb() << "kB. (peak: " << get_peak_memory_in_kb() << " kB.)");
        fsys::path vtree_path = dir / fsys::path(vtree_fname);
        Vtree* vtree = sdd_vtree_read(vtree_path.string().c_str());
        SddManager* manager = sdd_manager_new(vtree);

        fsys::path manager_path = dir / fsys::path(mng_fname);
        SddNode* node = sdd_read(manager_path.string().c_str(), manager);
        LPT_DEBUG("cout", "Done. SDD Size: " << sdd_size(node));

        if (sdd_node_is_false(node)) {
            std::cout << "Action " << schema_name << " has no applicable binding and will be ignored." << std::endl;
            continue;
        }

        // Load bookkeeping info for the schema
        std::vector<std::pair<VariableIdx, unsigned>> relevant;

        fsys::path atoms_path = dir / fsys::path(atoms_fname);
        std::ifstream is(atoms_path.string());
        if (is.fail()) {
            throw std::runtime_error("Could not open filename '" + atoms_fname + "'");
        }
        std::string line;
        while (std::getline(is, line)) {
            // each line is of the form "holding,c:5"
            std::vector<std::string> strings;
            boost::split(strings, line, boost::is_any_of(":"));
            assert(strings.size() == 2);
            auto sdd_varid = boost::lexical_cast<unsigned>(strings[1]);

            boost::split(strings, strings[0], boost::is_any_of(","));
            assert(!strings.empty()); // We'll have at least the id of the symbol
            auto symbol_id = info.getSymbolId(strings[0]);

            std::vector<object_id> constant_values;
            // Iterate but skipping first one
            for (std::size_t i = 1; i < strings.size(); ++i) constant_values.emplace_back(info.get_object_id(strings[i]));
            VariableIdx varid = info.resolveStateVariable(symbol_id, constant_values);

            relevant.emplace_back(varid, sdd_varid);
        }

        std::vector<std::vector<std::pair<object_id, unsigned>>> bindings;
        is = std::ifstream((dir / fsys::path(bindings_fname)).string());
        if (is.fail()) {
            throw std::runtime_error("Could not open filename '" + bindings_fname + "'");
        }

        while (std::getline(is, line)) {
            // i-th line is of the form "b:1,d:2,a:3,c:4" and corresponds to the bindings of parameter i of the schema
            std::vector<std::string> strings;
            boost::split(strings, line, boost::is_any_of(","));

            std::vector<std::pair<object_id, unsigned>> param_bindings;
            for (const auto& str:strings) {
                std::vector<std::string> substrings;
                boost::split(substrings, str, boost::is_any_of(":"));
                assert(substrings.size() == 2);
                const object_id& oid = info.get_object_id(substrings[0]);
                param_bindings.emplace_back(oid, boost::lexical_cast<unsigned>(substrings[1]));
            }

            bindings.push_back(std::move(param_bindings));
        }

        auto minimization_time = config.getOption<unsigned>("sdd.minimization_time", 10);
        if (minimization_time > 0) {
            ActionSchemaSDD::minimize_sdd(manager, node, minimization_time);
            // For debugging purposes:
            // std::cout << "Printing SDD to " << str(format("/home/gfrances/tmp/vtrees/%1%.sdd.dot") % schema_name) << std::endl;
            // sdd_save_as_dot(str(format("/home/gfrances/tmp/vtrees/%1%.sdd.dot") % schema_name).c_str(), node);
            // sdd_vtree_save_as_dot(str(format("/home/gfrances/tmp/vtrees/%1%.vtree.dot") % schema_name).c_str(), sdd_manager_vtree(manager));
        }

        sdds.push_back(std::make_shared<ActionSchemaSDD>(*schema, relevant, bindings, manager, vtree, node));
    }


    LPT_DEBUG("cout", "Mem. usage: " << get_current_memory_in_kb() << "kB. (peak: " << get_peak_memory_in_kb() << " kB.)");

    return sdds;
}

std::size_t ActionSchemaSDD::minimize_sdd(SddManager* manager, SddNode* node, unsigned time_limit) {
    auto t0 = aptk::time_used();
    std::size_t sz0 = sdd_size(node);
    LPT_INFO("cout", "Minimizing SDD with " << sz0 << " nodes for up to " << time_limit << " sec.");

    sdd_ref(node, manager);

    sdd_manager_set_vtree_search_time_limit((float) time_limit, manager);
    sdd_manager_minimize_limited(manager); // Go for it!
    // This should perform minimization until a certain threshold is reached (not 100% sure)
    //  sdd_manager_minimize(manager);

    double total_time = aptk::time_used() - t0;
    std::size_t sz1 = sdd_size(node);
    LPT_INFO("cout", "SDD minimization: " << sz0 << " -> " << sz1 << " nodes (" << std::fixed << std::setprecision(2) << (sz0-sz1) * 100 / sz1
                      << "% reduction). Actual minimization time: " << total_time << " sec.");

    // Minimization internally already triggers garbage collection (documented, and I've also tested it)
    sdd_deref(node, manager);

    return sz1;
}

ActionSchemaSDD::ActionSchemaSDD(const PartiallyGroundedAction& schema,
        std::vector<std::pair<VariableIdx, unsigned>> relevant,
        std::vector<std::vector<std::pair<object_id, unsigned>>> bindings,
        SddManager *manager, Vtree *vtree, SddNode *sddnode)
        : schema_(schema), sddmanager_(manager), vtree_(vtree), sddnode_(sddnode), relevant_(std::move(relevant)), bindings_(std::move(bindings))
{
}

ActionSchemaSDD::~ActionSchemaSDD() {
    // Note that we cannot simply `delete` the SDD objects here, as their implementation is hidden
    sdd_vtree_free(vtree_);
    sdd_manager_free(sddmanager_); // This should free memory from all nodes
}

SddNode* ActionSchemaSDD::conjoin_with(const State &state) const {
    SddNode* current = sddnode_;
    SddNode* literal = nullptr;
//    auto t0 = aptk::time_used();

//    LPT_DEBUG("cout", "\tConjoining with " << relevant_.size() << " relevant literals for given state.");
    for (const auto& elem:relevant_) {
        int atom = elem.second;
        if (!state.getValue(elem.first)) {
            // If the relevant atom doesn't hold in the given state, negate the literal.
            atom = -1 * atom;
        }
        literal = sdd_manager_literal(atom, sddmanager_);
        current = sdd_conjoin(current, literal, sddmanager_);
        // TODO Check if it is better to condition, not conjoin?
//        current = sdd_condition(atom, current, sddmanager_);
    }

//    LPT_DEBUG("cout", "\tConjoining time (sec):  " << aptk::time_used() - t0);
//    LPT_DEBUG("cout", "\tMem. usage: " << get_current_memory_in_kb() << "kB. (peak: " << get_peak_memory_in_kb() << " kB.)");

    return current;
}


SDDModel ActionSchemaSDD::collect_state_literals(const State &state) const {
    SDDModel literals(var_count()+1);

    for (const auto& elem:relevant_) {
        unsigned var = elem.second;
        if (!state.getValue(elem.first)) {
            // If the relevant atom doesn't hold in the given state, negate the literal.
            literals[var] = SDDModel::value_t::False;

        } else {
            literals[var] = SDDModel::value_t::True;
        }
    }

    return literals;
}

unsigned ActionSchemaSDD::var_count() const {
    return sdd_manager_var_count(sddmanager_);
}

std::vector<object_id> ActionSchemaSDD::get_binding_from_model(const SDDModel &model) {
    std::vector<object_id> values;
    values.reserve(bindings_.size());

    for (const auto& paramdata:bindings_) {
        for (const auto& var_obj:paramdata) {
            unsigned sdd_varid = var_obj.second;
            if (model[sdd_varid] == SDDModel::value_t::True) {
                values.push_back(var_obj.first);
                break;
            }
        }
    }

    assert(values.size() == bindings_.size());
    return values;
}


void ActionSchemaSDD::report_sdd_stats() const {
    printf("Live / dead sdd nodes: %zu / %zu\n", sdd_manager_live_size(sddmanager_), sdd_manager_dead_size(sddmanager_));
}

void ActionSchemaSDD::collect_sdd_garbage(SddNode* node) const {
//    printf("live / dead sdd nodes before garbage collection: %zu / %zu\n", sdd_manager_live_size(sddmanager_), sdd_manager_dead_size(sddmanager_));
    sdd_ref(sddnode_, sddmanager_);
    if (node) sdd_ref(node, sddmanager_);
    sdd_manager_garbage_collect(sddmanager_);
    sdd_deref(sddnode_, sddmanager_);
    if (node) sdd_deref(node, sddmanager_);
//    printf("Live / dead sdd nodes after garbage collection: %zu / %zu\n", sdd_manager_live_size(sddmanager_), sdd_manager_dead_size(sddmanager_));
}

RecursiveModelEnumerator::RecursiveModelEnumerator(SddManager* manager, SDDModel&& fixed)
        : sddmanager_(manager), nvars_(sdd_manager_var_count(manager)), fixed_(std::move(fixed)), cache_hits_(0), computed_nodes_(0) {
    assert(fixed_.size() == nvars_ + 1); // vars are 1-indexed
}

bool RecursiveModelEnumerator::node_is_false_in_fixed(SddNode* node) {
    if (!sdd_node_is_literal(node)) return false;
    const auto var = varid(sdd_node_literal(node));
    return fixed_[var] != SDDModel::value_t::Undefined && fixed_[var] != truth_value(node);
}

SDDModel SDDModel::merge_disjoint_models(const SDDModel& left, const SDDModel& right) {
    std::size_t size = left.size();
    if (size != right.size()) throw std::runtime_error("Unequal-sized SDD models cannot be merged");

    // Copy the left model and update the values from the right, checking for no overlap
    std::vector<value_t> res;
    res.reserve(size);
    res.push_back(value_t::Undefined); // variables are 1-indexed
    for (std::size_t i = 1; i < size; ++i) {
        const auto& lv = left[i];
        const auto& rv = right[i];

        if (lv != value_t::Undefined) {
            if (rv != value_t::Undefined) throw std::runtime_error("Attempted to merge overlapping SDD models");
            res.emplace_back(lv);
        } else {
            res.emplace_back(rv);
        }
    }

    return SDDModel(std::move(res));
}

DFSModelEnumerator::DFSModelEnumerator(SddManager* manager, SddNode* root, SDDModel&& fixed)
        : sddmanager_(manager), nvars_(sdd_manager_var_count(manager)), fixed_(std::move(fixed)), state_() {
    state_.reserve(nvars_);
    assert(fixed_.size() == nvars_ + 1); // vars are 1-indexed
    // auto vtree = sdd_node_is_true(root) ? sdd_manager_vtree(sddmanager_) : sdd_vtree_of(root);
    // state_.emplace_back(case_t::case0, (std::vector<SddNode*>){root}, vtree);
}

/*
SDDModel DFSModelEnumerator::next() {

    std::vector<SDDModel::value_t> values;

    while (!state_.empty()) {
        const auto& [subcase, nodes, vtree] = state_.back();
        assert(vtree);

//        auto vtree_left = sdd_vtree_left(vtree);
//        auto vtree_right = sdd_vtree_right(vtree);

        if (subcase == case_t::case0) { // Starting the ME algorithm
            assert(nodes.size() == 1);
            const auto& node = nodes[0];

            if (sdd_node_is_false(node)) throw std::runtime_error("False SDD has no models");


        }
    }

    // SDDModel(std::move(values))
    // We reached the end of the stack, all models have been explored, return the empty model to signal so
    return SDDModel(0);
}
*/

const RecursiveModelEnumerator::resultset_t&
RecursiveModelEnumerator::models_with_cache(SddNode* node, Vtree* vtree) {
    computed_nodes_++;
    auto key = std::make_pair(sdd_id(node), vtree);

//    auto res = computed_.emplace(key, vtree);
//    if (res.second) cache_hits_++;
//    return models_p(node, vtree);

    auto it = cache_.find(key);
    if (it != cache_.end()) {
        cache_hits_++;
        return it->second;
    } else {
        auto res = cache_.emplace(key, models(node, vtree));
        return res.first->second;
    }
}

std::vector<SDDModel> RecursiveModelEnumerator::models(SddNode* node) {
    if (sdd_node_is_false(node)) throw std::runtime_error("False SDD has no models");
    auto vtree = sdd_node_is_true(node) ? sdd_manager_vtree(sddmanager_) : sdd_vtree_of(node);
    auto selected = models(node, vtree);
    // std::cout << "SDD (" << sdd_size(node) << " nodes) has " << result.size() << " models. Cache hits: " << cache_hits_ << "/" << computed_nodes_ << std::endl;
//    std::cout << "A total of " << result_.size() << " models were computed, of which " << selected.size()
//              << " correspond to final total models" << std::endl;

    std::vector<SDDModel> totals;
    totals.reserve(selected.size());
    for (const auto& i:selected) {
        totals.push_back(result_[i]);
    }
    return totals;  // TODO Once this class is final, we should avoid this duplicate iteration and
                    //      simply return indexes to models
}


RecursiveModelEnumerator::resultset_t
RecursiveModelEnumerator::models(SddNode* node, Vtree* vtree) {
    auto vtree_left = sdd_vtree_left(vtree);
    auto vtree_right = sdd_vtree_right(vtree);

    if (sdd_vtree_is_leaf(vtree)) {
        unsigned var = sdd_vtree_var(vtree);
        assert (var > 0 && var <= nvars_);  // Variables in the SDD library range from 1 to numvars

        if (sdd_node_is_true(node)) {
            const auto& fixed_val = fixed_[var];
            if (fixed_val != SDDModel::value_t::Undefined) {
                return {register_model(var, fixed_val)};

            } else {
                return {register_model(var, SDDModel::value_t::True),
                        register_model(var, SDDModel::value_t::False)};
            }

        } else if (sdd_node_is_literal(node)) {
            const auto& fixed_val = fixed_[var];
            if (fixed_val != SDDModel::value_t::Undefined) {
                return {register_model(var, fixed_val)};

            } else {
                SDDModel::value_t value = truth_value(node);
                //            assert(fixed[var] == SDDModel::value_t::Undefined || fixed[var] == value); // Just in case
                return {register_model(var, value)};
            }
        }

    } else {
        if (sdd_node_is_true(node)) {
            return model_cross_product(node, node, vtree_left, vtree_right);

        } else if (sdd_vtree_of(node) == vtree) {
            assert(sdd_node_is_decision(node)); // Required by the documentation of `sdd_node_elements`

            // nodes is a C-style array of nodes containing (flat) pairs of (prime, sub), as described in the docs
            SddNode** nodes = sdd_node_elements(node);
            std::vector<index_t> result;
            // TODO Might be worth doing two passes to precompute the final size of result
            // TODO and reserve the appropriate space?

            auto nsize = 2*sdd_node_size(node);
            for (unsigned i=0; i < nsize; i += 2) {
                SddNode* prime = nodes[i];
                SddNode* sub = nodes[i+1];

                if (sdd_node_is_false(sub)) continue;

                if (node_is_false_in_fixed(sub) || node_is_false_in_fixed(prime)) continue;

                // For debugging purposes:
//                if (sdd_node_is_literal(prime) && sdd_node_literal(prime) == -7 &&
//                    sdd_node_is_literal(sub) && sdd_node_literal(sub) == -8) {
//                    std::cout << "Breakpoint" << std::endl;
//                }


                model_cross_product(prime, sub, vtree_left, vtree_right, result);
            }

            return result;


        } else {  // fill in gap in vtree
            auto truenode = sdd_manager_true(sddmanager_);

            // if Vtree.is_sub(node.vtree(), vtree.left()): [Python]
            if (sdd_vtree_is_sub(sdd_vtree_of(node), vtree_left)) {
                return model_cross_product(node, truenode, vtree_left, vtree_right);
            } else {
                return model_cross_product(truenode, node, vtree_left, vtree_right);
            }
        }
    }

    return {};
}


void RecursiveModelEnumerator::model_cross_product(SddNode* leftnode, SddNode* rightnode, Vtree* leftvt, Vtree* rightvt, resultset_t& output) {
    const resultset_t& left_models = models_with_cache(leftnode, leftvt);
    const resultset_t& right_models = models_with_cache(rightnode, rightvt);

    // increase vector capacity by expected number of models of the cross product
    output.reserve(output.size() + left_models.size() * right_models.size());

    for (const auto& l:left_models) {
        for (const auto& r:right_models) {
            output.emplace_back(register_model(SDDModel::merge_disjoint_models(result_[l], result_[r])));
        }
    }
}

RecursiveModelEnumerator::resultset_t
RecursiveModelEnumerator::model_cross_product(SddNode* leftnode, SddNode* rightnode, Vtree* leftvt, Vtree* rightvt) {
    resultset_t result;
    model_cross_product(leftnode, rightnode, leftvt, rightvt, result);
    return result;
}


RecursiveModelEnumerator::index_t RecursiveModelEnumerator::register_model(SDDModel&& model) {
    result_.push_back(std::move(model));
    return result_.size()-1;
}

RecursiveModelEnumerator::index_t RecursiveModelEnumerator::register_model(unsigned var, const SDDModel::value_t& val) {
    result_.emplace_back(nvars_+1);
    result_.back()[var] = val;
//    return &result_.back();
    return result_.size()-1;
}




} // namespace