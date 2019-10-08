
#include <fs/core/state.hxx>
#include <fs/core/actions/sdd_action_iterator.hxx>
#include <fs/core/actions/action_id.hxx>
#include <fs/core/languages/fstrips/formulae.hxx>
#include <fs/core/utils/atom_index.hxx>
#include <sdd/sddapi.hxx>

namespace fs0 {

    LiftedActionID* generate_lifted_action_id_from_sdd_model(const SDDModel& model) {
        throw std::runtime_error("TO BE IMPLEMENTED");
        return nullptr;
//        return new LiftedActionID(&_action, build_binding_from_solution(solution));
    }

    SDDActionIterator::SDDActionIterator(const State& state, const std::vector<std::shared_ptr<ActionSchemaSDD>>& sdds, const AtomIndex& tuple_index) :
            state_(state), sdds_(sdds)
    {}

    SDDActionIterator::Iterator::Iterator(const State& state, const std::vector<std::shared_ptr<ActionSchemaSDD>>& sdds, unsigned currentIdx) :
            state_(state),
            sdds_(sdds),
            current_sdd_idx_(currentIdx),
            current_sdd_(nullptr),
            _action(nullptr)
    {
        advance();
    }

    SDDActionIterator::Iterator::~Iterator() {
        delete _action;
    }

    void SDDActionIterator::Iterator::advance() {
        for (; current_sdd_idx_ < sdds_.size(); ++current_sdd_idx_) {
            const std::shared_ptr<ActionSchemaSDD>& schema_sdd = sdds_[current_sdd_idx_];

            if (!current_sdd_) {
                assert (!model_iterator_);

                // Create the SDD corresponding to the current action schema index conjoined with the current state.
                current_sdd_ = schema_sdd->conjoin_with(state_);
                if (!current_sdd_ || sdd_node_is_false(current_sdd_)) {// no applicable ground action for this schema

                    // No delete, as SDD library has custom mem management.
                    // Should get deleted when garbage collection or when manager gets deleted
                    current_sdd_ = nullptr;
                    continue;
                }

                model_iterator_ = new SDDModelIterator();
            }



            SDDModel* model = model_iterator_->next();
            if (!model) {  // We have already explored all solutions to the SDD
                // TODO Perhaps garbage-collect here

                current_sdd_ = nullptr;
                delete model_iterator_; model_iterator_ = nullptr;
                continue;
            }

            delete _action;
            _action = generate_lifted_action_id_from_sdd_model(*model);
            delete model;
            break;
        }
    }


} // namespaces
