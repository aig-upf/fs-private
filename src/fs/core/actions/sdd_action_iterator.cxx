
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

                // TODO custom_me no longer means custom me - fix this :-)
                if (custom_me_) {
                    RecursiveModelEnumerator enumerator(schema_sdd.manager(), schema_sdd.collect_state_literals(state_));
                    current_resultset_ = enumerator.models(schema_sdd.node());

                } else {
                    RecursiveModelEnumerator2 enumerator(schema_sdd.manager(), schema_sdd.collect_state_literals(state_));
                    current_resultset_ = enumerator.models(schema_sdd.node());
                }
//              std::cout << current_resultset_.size() << " models were actually retrieved" << std::endl;

                current_models_computed_ = true;
                current_resultset_idx_ = 0;
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
