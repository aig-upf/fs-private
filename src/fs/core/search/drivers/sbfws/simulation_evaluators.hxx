
#pragma once

#include <utility>
#include <utility>
#include <vector>
#include <memory>

#include <fs/core/search/drivers/sbfws/config.hxx>
#include <fs/core/languages/fstrips/effects.hxx>
#include <fs/core/state.hxx>
#include <fs/core/heuristics/unsat_goal_atoms.hxx>
#include <fs/core/actions/actions.hxx>
#include <fs/core/languages/fstrips/terms.hxx>
#include <fs/core/problem_info.hxx>


namespace fs0::bfws {


inline uint32_t _combine_indexes(uint32_t k, uint32_t q, uint32_t p, uint32_t Q) {
    return k*Q*Q + q*Q + p;
}


template<typename NodeT>
class SimulationEvaluatorI {
public:
    virtual unsigned evaluate(NodeT& node) = 0;

    virtual void reset() = 0;

    virtual std::vector<bool> reached_atoms() const = 0;

    virtual void info() const {}
};

template<typename NodeT, typename FeatureSetT, typename NoveltyEvaluatorT>
class SimulationEvaluator : public SimulationEvaluatorI<NodeT> {
protected:
    //! The set of features used to compute the novelty
    const FeatureSetT& _features;

    //! A single novelty evaluator will be in charge of evaluating all nodes
    std::unique_ptr<NoveltyEvaluatorT> _evaluator;

public:
    SimulationEvaluator(const FeatureSetT& features, NoveltyEvaluatorT* evaluator) :
            _features(features),
            _evaluator(evaluator) {}

    ~SimulationEvaluator() = default;

    unsigned evaluate(NodeT& node) override {
        if (node.parent) {
            // Important: the novel-based computation works only when the parent has the same novelty type and thus goes against the same novelty tables!!!
            node._w = _evaluator->evaluate(_features.evaluate(node.state), _features.evaluate(node.parent->state));
        } else {
            node._w = _evaluator->evaluate(_features.evaluate(node.state));
        }

        return node._w;
    }

    std::vector<bool> reached_atoms() const override {
        std::vector<bool> atoms;
        _evaluator->mark_atoms_in_novelty1_table(atoms);
        return atoms;
    }

    void reset() override {
        _evaluator->reset();
    }
};


template<
        typename NodeT,
        typename FeatureSetT,
        typename NoveltyEvaluatorT
>
class AchieverNoveltyEvaluator : public SimulationEvaluatorI<NodeT> {
public:
    using FeatureValueT = typename NoveltyEvaluatorT::FeatureValueT;

public:
    AchieverNoveltyEvaluator(const Problem& problem,
            const FeatureSetT& features,
            const std::vector<PlainOperator>& operators,
            const std::vector<std::vector<unsigned>>& achievers) :
            _featureset(features),
            operators_(operators),
            achievers_(achievers),
            _search_novelty_factory(problem, SBFWSConfig::NoveltyEvaluatorType::Adaptive,
                    _featureset.uses_extra_features(), 1)
    {
    }

    ~AchieverNoveltyEvaluator() {
        for (auto& elem:tables_) delete elem.second;
    };

    void info() const override {
        LPT_INFO("cout", "Simulation - Total num. novelty tables created: " << tables_.size());
    }



    unsigned evaluate(NodeT& node) override {
        const State& state = node.state;
        unsigned min_nov = std::numeric_limits<unsigned>::max();
        for (unsigned q = 0; q < state.numAtoms(); ++q) {
            unsigned k = compute_achiever_satisfaction_factor(node.state, q);
            auto& table = novelty_table(q, k);
            auto nov = table.evaluate(_featureset.evaluate(node.state), 1);
//            std::cout << "Novelty in table for q=" << q << ": " << nov << std::endl;
            min_nov = std::min(min_nov, nov);
        }

        return min_nov;
    }

    //! Return the "achiever satisfaction factor" #q(s) for the given state s and atom q,
    //! which is the max. k such that there is a ground action that achieves q and has
    //! k satisfied preconditions in state s
    unsigned compute_achiever_satisfaction_factor(const State& state, unsigned var) const {
        unsigned max_precs_achieved = 0;
        for (const auto& actionidx:achievers_[var]) {
            unsigned achieved = 0;
            for (const auto& pre:operators_[actionidx].precondition_) {
                if (state.getValue(pre.first) == pre.second) {
                    achieved++;
                }
            }
            max_precs_achieved = std::max(max_precs_achieved, achieved);
        }
//        std::cout << "k=" << max_precs_achieved << "; " << state << std::endl;

        return max_precs_achieved;
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

    std::vector<bool> reached_atoms() const override {
        throw std::runtime_error("Unimplemented");
    }

    void reset() override {
        for (auto& elem:tables_) delete elem.second;
        tables_.clear();
    }

protected:
    const FeatureSetT& _featureset;

    std::vector<PlainOperator> operators_;

    std::vector<std::vector<unsigned>> achievers_;

    const NoveltyFactory<FeatureValueT> _search_novelty_factory;

    using TableKeyT = std::pair<unsigned, unsigned>;
    std::unordered_map<TableKeyT, NoveltyEvaluatorT*, boost::hash<TableKeyT>> tables_;

};

template<
        typename NodeT,
        typename FeatureSetT,
        typename NoveltyEvaluatorT
>
class BitvectorAchieverNoveltyEvaluator : public AchieverNoveltyEvaluator<NodeT, FeatureSetT, NoveltyEvaluatorT> {

public:
    using BaseT = AchieverNoveltyEvaluator<NodeT, FeatureSetT, NoveltyEvaluatorT>;

    BitvectorAchieverNoveltyEvaluator(const Problem& problem,
            const FeatureSetT& features,
            const std::vector<PlainOperator>& operators,
            const std::vector<std::vector<unsigned>>& achievers,
            unsigned max_precondition_size,
            unsigned nvars) :
            BaseT(problem, features, operators, achievers),
            max_precondition_size_(max_precondition_size),
            nvars_(nvars),
            seen_(nvars*nvars*(max_precondition_size+1), false)
//            reached_(nvars, false)
    {

    }

    unsigned evaluate(NodeT& node) override {
        const State& state = node.state;

        const auto& valuation = this->_featureset.evaluate(state);
        assert(state.numAtoms() == nvars_);
        assert(valuation.size() == nvars_);

        bool is_novel = false;

        for (unsigned q = 0; q < nvars_; ++q) {
            unsigned k = this->compute_achiever_satisfaction_factor(node.state, q);

            bool exists_novel_tuple = false;
            for (unsigned var_index = 0; var_index < nvars_; ++var_index) {
                const auto& value = valuation[var_index];
                if (value == 0) continue; // ignore negative atoms

                unsigned atom_index = _combine_indexes(k, q, var_index, nvars_);
                std::vector<bool>::reference ref = seen_.at(atom_index);
                if (!ref) { // The tuple is new
                    ref = true;
                    exists_novel_tuple = true;
                }
            }

            is_novel |= exists_novel_tuple;
//            std::cout << "Novelty in table for q=" << q << ": " << nov << std::endl;
        }

        return is_novel ? 1 : std::numeric_limits<unsigned>::max();
    }


protected:
    unsigned max_precondition_size_;
    unsigned nvars_;
    std::vector<bool> reached_;
    std::vector<bool> seen_;
};

//! Factory method
template<
        typename NodeT,
        typename FeatureSetT,
        typename NoveltyEvaluatorT
>
std::unique_ptr<AchieverNoveltyEvaluator<NodeT, FeatureSetT, NoveltyEvaluatorT>>
create_achiever_evaluator(const Problem& problem,
        const FeatureSetT& features,
        const std::vector<PlainOperator>& operators,
        unsigned long max_table_size) {

    const auto& info = ProblemInfo::getInstance();
    unsigned nvars = info.getNumVariables();
    std::vector<std::vector<unsigned>> achievers(nvars);

    std::size_t max_precondition_size = 0;
    for (std::size_t actionidx = 0, n = operators.size(); actionidx < n; ++actionidx) {
        const auto& op = operators[actionidx];

        max_precondition_size = std::max(max_precondition_size, op.precondition_.size());

        for (const auto& eff:op.effects_) {
            if (eff.second == object_id::TRUE) {
                achievers[eff.first].push_back(actionidx);
            }
        }
    }

    unsigned long expected_table_entries = nvars*nvars*(max_precondition_size+1);
    unsigned long expected_table_size_in_kb = expected_table_entries / (8 * 1024); // size in kilobytes

    LPT_INFO("cout", "Max. precondition size: " << max_precondition_size);
    LPT_INFO("cout", "Num. state variables: " << nvars);
    LPT_INFO("cout", "Expected table size: " << expected_table_size_in_kb << "KB (entries: " << expected_table_entries << ", max. size: " << max_table_size <<")");

    using ET = BitvectorAchieverNoveltyEvaluator<NodeT, FeatureSetT, NoveltyEvaluatorT>;
    return std::make_unique<ET>(problem, features, operators, achievers, max_precondition_size, nvars);
}

} // namespaces