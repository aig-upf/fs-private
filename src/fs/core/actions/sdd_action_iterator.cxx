
#include <fs/core/state.hxx>
#include <fs/core/actions/sdd_action_iterator.hxx>
#include <fs/core/actions/action_id.hxx>
#include <fs/core/actions/actions.hxx>
#include <fs/core/languages/fstrips/formulae.hxx>
#include <fs/core/utils/atom_index.hxx>
#include <sdd/sddapi.hxx>

#include <lapkt/tools/logging.hxx>

namespace fs0 {

    SDDActionIterator::SDDActionIterator(const State& state, const std::vector<std::shared_ptr<ActionSchemaSDD>>& sdds, const AtomIndex& tuple_index, bool custom_me) :
            state_(state), sdds_(sdds), custom_me_(custom_me)
    {}

    SDDActionIterator::Iterator::Iterator(const State& state, const std::vector<std::shared_ptr<ActionSchemaSDD>>& sdds, unsigned currentIdx, bool custom_me) :
            state_(state),
            sdds_(sdds),
            current_sdd_idx_(currentIdx),
            current_sdd_(nullptr),
            current_models_computed_(false),
            _action(nullptr),
            current_resultset_(),
            custom_me_(custom_me)
    {
        advance();
    }

    SDDActionIterator::Iterator::~Iterator() {
        delete _action;
    }

    void SDDActionIterator::Iterator::advance() {
        for (; current_sdd_idx_ < sdds_.size(); ++current_sdd_idx_) {
            ActionSchemaSDD& schema_sdd = *sdds_[current_sdd_idx_];

            if (!current_models_computed_) {
                assert (current_resultset_.empty());

                /*
                if (!custom_me_) {
                    // LPT_DEBUG("cout", "Conjoining SDD " << schema_sdd.get_schema().getName() << " with state...");
                    // Create the SDD corresponding to the current action schema index conjoined with the current state.
                    current_sdd_ = schema_sdd.conjoin_with(state_);
                    if (!current_sdd_ ||
                        sdd_node_is_false(current_sdd_)) { // no applicable ground action for this schema

                        // No delete, as SDD library has custom mem management.
                        // This should collect the garbage from current_sdd_ (i.e. closest thing to deleting the pointer),
                        // as it will garbage-collect without referencing the current_sdd node
                        schema_sdd.collect_sdd_garbage();
                        current_sdd_ = nullptr;
//                    LPT_DEBUG("cout", "Conjoined SDD has no groundings!");
                        continue;
                    }

                    //                auto wmc_manager = wmc_manager_new(current_sdd_, 0, schema_sdd.manager());
//                double wmc = wmc_propagate(wmc_manager);
//                std::cout << "Action schema " << schema_sdd.get_schema().getName() << " has " << wmc << " models... " << std::flush;
//                LPT_DEBUG("cout", "Conjoined SDD has " <<  sdd_size(current_sdd_) << " nodes");
                }
                */


                // TODO custom_me no longer means custom me - fix this :-)
                if (custom_me_) {
                    RecursiveModelEnumerator enumerator(schema_sdd.manager(), schema_sdd.collect_state_literals(state_));
                    current_resultset_ = enumerator.models(schema_sdd.node());

                } else {
                    // Just pass an empty SDDModel, i.e. with no value fixed
//                    RecursiveModelEnumerator enumerator(schema_sdd.manager(), SDDModel(schema_sdd.var_count()+1));
//                    current_resultset_ = enumerator.models(current_sdd_);
                    RecursiveModelEnumerator2 enumerator(schema_sdd.manager(), schema_sdd.collect_state_literals(state_));
                    current_resultset_ = enumerator.models(schema_sdd.node());
                }

                current_models_computed_ = true;
//                std::cout << current_resultset_.size() << " models were actually retrieved" << std::endl;
                current_resultset_idx_ = 0;
                // This should collect the garbage from current_sdd_ (i.e. closest thing to deleting the pointer),
                // as it will garbage-collect without referencing the current_sdd node
//                schema_sdd.collect_sdd_garbage(); // This doesnt make sense if we don't modify the SDD with data from the current state
            }

            if (current_resultset_idx_ < current_resultset_.size()) {
                const auto& model = current_resultset_[current_resultset_idx_];

                delete _action;
                auto grounding = schema_sdd.get_binding_from_model(model);
                auto gr_size = grounding.size();

                _action = new LiftedActionID(&schema_sdd.get_schema(),
                        Binding(std::move(grounding), std::vector<bool>(gr_size, true)));

                ++current_resultset_idx_;
                return;
            }

            // At this point we have explored all solutions to the current action-schema SDD
            current_sdd_ = nullptr;
            current_models_computed_ = false;
            current_resultset_.clear();
            current_resultset_idx_ = 0;
        }
    }


} // namespaces
