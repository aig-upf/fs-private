
#include <fs/core/problem.hxx>
#include <fs/core/search/drivers/sbfws/base.hxx>
#include <fs/core/utils/config.hxx>
#include <fs/core/problem_info.hxx>

namespace fs0::bfws {

template <typename FeatureValueT>
NoveltyFactory<FeatureValueT>::
NoveltyFactory(const Problem& problem, SBFWSConfig::NoveltyEvaluatorType desired_evaluator_t, bool use_extra_features, unsigned max_expected_width) :
    _problem(problem), _indexer(_problem.get_tuple_index()), _desired_evaluator_t(desired_evaluator_t)
{
    const Config& config = Config::instance(); // TODO - Remove the singleton use and inject the config here by other means
    _ignore_neg_literals = config.getOption<bool>("ignore_neg_literals", true);

    _chosen_evaluator_t.resize(max_expected_width+1, ChosenEvaluatorT::Generic);

    // If we're using extra novelty features, we cannot use the specialized evaluators as of now.
    if (use_extra_features) {
        return;
    }

    for (unsigned w = 1; w <= max_expected_width; ++w) {

        // If asked for, check first if a specialized Atom-Evaluator is suitable,
        // i.e. because its memory requirements are not too high.
        if (can_use_atom_evaluator(w)) {
            if (w == 1) {
                LPT_INFO("search", "NOVELTY EVALUATION: Chosen a specialized width-1 atom evaluator");
                _chosen_evaluator_t[w] = ChosenEvaluatorT::W1Atom;
            }
            else {
                LPT_INFO("search", "NOVELTY EVALUATION: Chosen a specialized width-2 atom evaluator");
                _chosen_evaluator_t[w] = ChosenEvaluatorT::W2Atom;
            }
        } else {
            LPT_INFO("search", "NOVELTY EVALUATION: Chosen a generic evaluator");
            _chosen_evaluator_t[w] = ChosenEvaluatorT::Generic;
        }
    }
}

template <typename FeatureValueT>
bool NoveltyFactory<FeatureValueT>::
can_use_atom_evaluator(unsigned width) const {
    if (width > 2) return false;

    unsigned num_atom_indexes = _indexer.num_indexes();

    // We want to make sure that the size of the novelty table is smaller than a certain pre-defined constant
    if (width == 1) {
        return W1AtomEvaluator::expected_size(num_atom_indexes) < 1000000; // i.e. max 1MB per novelty-1 table.

    } else {
        // Else the desired width is 2
        return W2AtomEvaluator::expected_size(num_atom_indexes) < 10000000; // i.e. max 10MB per novelty-2 table.
    }
}

template <typename FeatureValueT>
typename NoveltyFactory<FeatureValueT>::NoveltyEvaluatorT*
NoveltyFactory<FeatureValueT>::create_evaluator(unsigned width) const {

    auto ev_type = _chosen_evaluator_t[width];
    if (ev_type ==  ChosenEvaluatorT::W1Atom) {
        return new W1AtomEvaluator(_indexer, _ignore_neg_literals);

    } else if (ev_type ==  ChosenEvaluatorT::W2Atom) {
        return new W2AtomEvaluator(_indexer, _ignore_neg_literals);

    } else if (ev_type ==  ChosenEvaluatorT::Generic) {
        return new GenericEvaluator(width);

    } else {
        throw std::runtime_error("Unknown evaluator type");
    }
}

template <typename FeatureValueT>
typename NoveltyFactory<FeatureValueT>::NoveltyEvaluatorT*
NoveltyFactory<FeatureValueT>::create_compound_evaluator(unsigned max_width) const {

    // This is a very basic strategy, but others more sophisticated can be easily devised.
    // If the decision algorithm considers that width-2 computations can be performed
    // with the optimized evaluator, then we choose the compoung evaluator; otherwise
    // simply choose the generic

    if (max_width == 1) {
        return create_evaluator(1);
    }

    if (max_width == 2 && _chosen_evaluator_t[2] ==  ChosenEvaluatorT::W2Atom) {
        return new CompoundAtomEvaluator(_indexer, _ignore_neg_literals);
    }
    return new GenericEvaluator(max_width);
}


// explicit instantiations
template class NoveltyFactory<bool>;
template class NoveltyFactory<int>;





} // namespaces
