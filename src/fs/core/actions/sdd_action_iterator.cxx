
#include <fs/core/state.hxx>
#include <fs/core/actions/sdd_action_iterator.hxx>
#include <fs/core/actions/propositional_actions.hxx>
#include <fs/core/utils/atom_index.hxx>
#include <sdd/sddapi.hxx>

#include <lapkt/tools/logging.hxx>

namespace fs0 {

    SDDActionIterator::SDDActionIterator(const State& state, const std::vector<std::shared_ptr<ActionSchemaSDD>>& sdds, const AtomIndex& tuple_index) :
            state_(state), sdds_(sdds)
    {}

    SDDActionIterator::Iterator::Iterator(const State& state, const std::vector<std::shared_ptr<ActionSchemaSDD>>& sdds, unsigned currentIdx) :
            state_(state),
            sdds_(sdds),
            current_sdd_idx_(currentIdx),
            current_sdd_(nullptr),
            current_models_computed_(false),
            current_model_(nullptr),
            current_resultset_()
    {
        advance();
    }

    void SDDActionIterator::Iterator::advance() {
        for (; current_sdd_idx_ < sdds_.size(); ++current_sdd_idx_) {
            ActionSchemaSDD& schema_sdd = *sdds_[current_sdd_idx_];

            if (!current_models_computed_) {
                assert (current_resultset_.empty());

                RecursiveModelEnumerator enumerator(schema_sdd.manager(), schema_sdd.collect_state_literals(state_));
                current_resultset_ = enumerator.models(schema_sdd.node());

//              std::cout << current_resultset_.size() << " models were actually retrieved" << std::endl;

                current_models_computed_ = true;
                current_resultset_idx_ = 0;
            }

            if (current_resultset_idx_ < current_resultset_.size()) {
                current_model_ = &current_resultset_[current_resultset_idx_];  // Store the model to generate the action if necessary
                ++current_resultset_idx_;
                return;
            }

            // At this point we have explored all solutions to the current action-schema SDD
            current_sdd_ = nullptr;
            current_model_ = nullptr;
            current_models_computed_ = false;
            current_resultset_.clear();
            current_resultset_idx_ = 0;
        }
    }

SchematicActionID
SDDActionIterator::Iterator::operator*() const {
    ActionSchemaSDD& schema_sdd = *sdds_[current_sdd_idx_];
    return SchematicActionID(&schema_sdd.schematic_action(), schema_sdd.get_binding_from_model(*current_model_));
}


} // namespaces
