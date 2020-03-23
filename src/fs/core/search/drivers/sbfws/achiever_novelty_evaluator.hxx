
#pragma once

#include <unordered_map>
#include <fs/core/state.hxx>
#include <fs/core/problem.hxx>
#include <fs/core/search/drivers/sbfws/base.hxx>
#include "stats.hxx"
#include "config.hxx"

namespace fs0::bfws {

template<
    typename FeatureSetT,
    typename NoveltyEvaluatorT,
    typename NodeT
>
class AchieverNoveltyEvaluator {
public:
    using FeatureValueT = typename NoveltyEvaluatorT::FeatureValueT;


protected:
    const FeatureSetT& _featureset;

    const NoveltyFactory<FeatureValueT> _search_novelty_factory;

//    //! A counter to count the number of unsatisfied goals
//    UnsatisfiedGoalAtomsCounter _unsat_goal_atoms_heuristic;

    BFWSStats& _stats;

    SBFWSConfig _sbfwsconfig;


public:
    AchieverNoveltyEvaluator(const Problem& problem, const FeatureSetT& features, const SBFWSConfig& config, BFWSStats& stats) :
            _featureset(features),
            _search_novelty_factory(problem, config.evaluator_t, _featureset.uses_extra_features(), config.search_width),
//            _unsat_goal_atoms_heuristic(problem),
            _stats(stats),
            _sbfwsconfig(config)
    {
        const AtomIndex& atomidx = problem.get_tuple_index();
        num_atoms_ = atomidx.size();
        for (unsigned atom = 0; atom < num_atoms_; ++atom) {

        }

    }

    ~AchieverNoveltyEvaluator() {
        for (auto& elem:tables_) for (auto& p:elem) delete p.second;
    };


    unsigned evaluate(NodeT& node) {
        unsigned min_nov = std::numeric_limits<unsigned>::max();
        for (unsigned q = 0; q < num_atoms_; ++q) {
            unsigned k = compute_achiever_satisfaction_factor(node->state, q);
            auto& table = novelty_table(q, k);
            min_nov = std::min(min_nov, table.evaluate(_featureset.evaluate(node.state), 1));
        }

        return min_nov;
    }

    //!
    unsigned compute_achiever_satisfaction_factor(const State& state, unsigned atom) const {
        assert(0);
        return 0;
    }

    //! Return the novelty table that corresponds to given atom and max. achiever satisfaction factor.
    //! If that table had not yet been created, create it and return it.
    NoveltyEvaluatorT& novelty_table(unsigned atom, unsigned k) {
        const auto& key = std::make_pair(atom, k);
        auto it = tables_.find(key);
        if (it == tables_.end()) {
            auto inserted = tables_.insert(std::make_pair(key, _search_novelty_factory.create_evaluator(1)));
            it = inserted.first;
        }
        return *(it->second);
    }

protected:
    unsigned num_atoms_;

    std::unordered_map<std::pair<unsigned, unsigned>, NoveltyEvaluatorT*> tables_;


};

} // namespaces