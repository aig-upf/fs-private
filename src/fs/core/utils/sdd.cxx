
#include <fs/core/problem_info.hxx>
#include <fs/core/actions/actions.hxx>
#include <fs/core/utils/sdd.hxx>
#include <fs/core/utils/lexical_cast.hxx>

#include <sdd/sddapi.hxx>

#include <boost/filesystem.hpp>   // includes all needed Boost.Filesystem declarations
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>


#include <iostream>
#include <memory>

#include <fs/core/state.hxx>

namespace fsys = boost::filesystem;
using boost::format;

namespace fs0 {

//! Loads from disk all SDDs in the given directory (one per action schema)
std::vector<std::shared_ptr<ActionSchemaSDD>>
load_sdds_from_disk(const std::vector<const PartiallyGroundedAction*>& schemas, const std::string& dir) {
    const ProblemInfo& info = ProblemInfo::getInstance();
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
        std::cout << "Reading SDD files corresponding to \"" << schema_name << "\"...";
        fsys::path vtree_path = dir / fsys::path(vtree_fname);
        Vtree* vtree = sdd_vtree_read(vtree_path.string().c_str());
        SddManager* manager = sdd_manager_new(vtree);

        fsys::path manager_path = dir / fsys::path(mng_fname);
        SddNode* node = sdd_read(manager_path.string().c_str(), manager);
        std::cout << "Done. SDD Size: " << sdd_size(node) << std::endl;

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

        sdds.push_back(std::make_shared<ActionSchemaSDD>(*schema, relevant, bindings, manager, vtree, node));
    }

    return sdds;
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
    for (const auto& elem:relevant_) {
        int atom = elem.second;
        if (!state.getValue(elem.first)) {
            // If the relevant atom doesn't hold in the given state, negate the literal.
            atom = -1 * atom;
        }
        literal = sdd_manager_literal(atom, sddmanager_);
        current = sdd_conjoin(current, literal, sddmanager_);
    }

    // TODO Perhaps garbage-collect here?

    return current;
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

        return values;
    }

    std::vector<SDDModel> SDDModelEnumerator::models(SddNode* node, Vtree* vtree) {
    // This is a direct translation of the `models()` method of the Python PySDD API:
    // <https://github.com/wannesm/PySDD/blob/5a301a9/pysdd/sdd.pyx#L247>

    if (sdd_node_is_false(node)) throw std::runtime_error("False SDD has no models");

    if (vtree == nullptr) {
        vtree = sdd_node_is_true(node) ? sdd_manager_vtree(sddmanager_) : sdd_vtree_of(node);
    }

    auto vtree_left = sdd_vtree_left(vtree);
    auto vtree_right = sdd_vtree_right(vtree);


    if (sdd_vtree_is_leaf(vtree)) {
        unsigned var = sdd_vtree_var(vtree);
        assert (var > 0 && var <= nvars_);  // Variables in the SDD library range from 1 to numvars

        if (sdd_node_is_true(node)) {
            std::vector<SDDModel> result;
            result.emplace_back(nvars_+1);
            result.back()[var] = SDDModel::value_t::True;

            result.emplace_back(nvars_+1);
            result.back()[var] = SDDModel::value_t::False;
            return result;

        } else if (sdd_node_is_literal(node)) {
            SDDModel::value_t value = sdd_node_literal(node) < 0 ? SDDModel::value_t::False : SDDModel::value_t::True;
            std::vector<SDDModel> result;
            result.emplace_back(nvars_+1);
            result.back()[var] = value;
            return result;
        }

    } else {
        if (sdd_node_is_true(node)) {
            return model_cross_product(node, node, vtree_left, vtree_right);

        } else if (sdd_vtree_of(node) == vtree) {
            assert(sdd_node_is_decision(node)); // Required by the documentation of `sdd_node_elements`

            // nodes is a C-style array of nodes containing (flat) pairs of (prime, sub), as described in the docs
            SddNode** nodes = sdd_node_elements(node);
            std::vector<SDDModel> result;
            // TODO Might be worth doing two passes to precompute the final size of result
            // TODO and reserve the appropriate space?

            auto nsize = 2*sdd_node_size(node);
            for (unsigned i=0; i < nsize; i += 2) {
                SddNode* prime = nodes[i];
                SddNode* sub = nodes[i+1];

                if (sdd_node_is_false(sub)) continue;

                model_cross_product(prime, sub, vtree_left, vtree_right, result);
            }

            return result;


        } else {  // fill in gap in vtree
            auto truenode = sdd_manager_true(sddmanager_);

            if (sdd_vtree_is_sub(vtree, vtree_left)) {
                return model_cross_product(node, truenode, vtree_left, vtree_right);
            } else {
                return model_cross_product(truenode, node, vtree_left, vtree_right);
            }
        }
    }

    throw std::runtime_error("Shouldn't get here :-)");
}

void SDDModelEnumerator::model_cross_product(SddNode* leftnode, SddNode* rightnode, Vtree* leftvt, Vtree* rightvt, std::vector<SDDModel>& output) {
    auto left_models = models(leftnode, leftvt);
    auto right_models = models(rightnode, rightvt);

    // increase vector capacity by expected number of models of the cross product
    output.reserve(output.size() + left_models.size() * right_models.size());

    for (const auto& l:left_models) {
        for (const auto& r:right_models) {
            output.emplace_back(SDDModel::merge_disjoint_models(l, r));
        }
    }
}

std::vector<SDDModel> SDDModelEnumerator::model_cross_product(SddNode* leftnode, SddNode* rightnode, Vtree* leftvt, Vtree* rightvt) {
    std::vector<SDDModel> result;
    model_cross_product(leftnode, rightnode, leftvt, rightvt, result);
    return result;
}

SDDModelEnumerator::SDDModelEnumerator(SddManager* manager)
    : sddmanager_(manager), nvars_(sdd_manager_var_count(manager)) {}

SDDModel SDDModel::merge_disjoint_models(const SDDModel& left, const SDDModel& right) {
    std::size_t size = left.size();
    if (size != right.size()) throw std::runtime_error("Unequal-sized SDD models cannot be merged");

    // Copy the left model and update the values from the right, checking for no overlap
    std::vector<value_t> res;
    res.reserve(size);
    for (std::size_t i = 0; i < size; ++i) {
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

} // namespace