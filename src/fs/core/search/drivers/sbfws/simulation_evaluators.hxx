
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


struct AchieverNoveltyConfiguration {
    AchieverNoveltyConfiguration(
            unsigned long max_table_size,
            bool break_on_first_novel) :
        max_table_size_(max_table_size),
        break_on_first_novel_(break_on_first_novel)
    {}

    unsigned long max_table_size_;
    bool break_on_first_novel_;
};


template <typename T>
inline unsigned get_action_id(const T& o) { throw std::runtime_error("Unimplemented"); }

template <>
inline unsigned get_action_id<unsigned>(const unsigned& o) { return o; }


template <typename T>
inline const std::vector<bool>& get_valuation(const T& o) { throw std::runtime_error("Unimplemented"); }

template <>
inline const std::vector<bool>& get_valuation<std::vector<bool>>(const std::vector<bool>& o) { return o; }

// A helper. Combine indexes in ranges
// k \in [0..k] ("number of preconditions to go")
// q, p \in [0..Q-1]  ("atom indexes")
inline std::size_t _combine_indexes(std::size_t k, std::size_t q, std::size_t p, std::size_t Q) {
    return k*Q*Q + q*Q + p;
}

inline std::size_t _combine_rcontext_indexes(std::size_t p, std::size_t num_true_atoms, std::size_t Q) {
    return num_true_atoms*Q + p;
}


template<
        typename NodeT,
        typename FeatureSetT,
        typename NoveltyEvaluatorT
>
class BitvectorAchieverNoveltyEvaluator : public SimulationEvaluatorI<NodeT> {

public:
    BitvectorAchieverNoveltyEvaluator(
            const AtomIndex& atom_idx,
            const FeatureSetT& features,
            const std::vector<SASPlusOperator>& operators,
            const std::vector<std::vector<unsigned>>& op_adds,
            const std::vector<std::vector<unsigned>>& achievers,
            unsigned max_precondition_size,
            unsigned nvars,
            const AchieverNoveltyConfiguration& config) :
        atom_idx_(atom_idx),
        n_(atom_idx_.size()),
        _featureset(features),
        operators_(operators),
        op_adds_(op_adds),
        achievers_(achievers),
        config_(config),
        max_precondition_size_(max_precondition_size),
        nvars_(nvars),
        reached_(n_, false),
        seen_(delta_table_size(), false),
        r_tables_(r_table_size(), false)
    {
//        std::unordered_set<unsigned> idxs;
//        for (unsigned q = 0; q < n_; ++q) {
//            for (unsigned p = 0; p < n_; ++p) {
//                for (unsigned k = 0; k <= max_precondition_size_+1; ++k) {
//                    for (unsigned nat = 0; nat < n_; ++nat) {
//                        auto idx = _combine_indexes(k, q, p, nat, n_);
////                        std::cout << " " << k << " " << q << " " << p << ": " << idx << std::endl;
//                        auto it = idxs.insert(idx);
//                        if (!it.second) throw std::runtime_error("WHAT!");
//                        if (idx >= seen_.size()) throw std::runtime_error("WHAT2!");
//                    }
//
//                }
//            }
//        }
//        std::cout << "Theoretical size: " << delta_table_size() << std::endl;
//        std::cout << "Actual size: " << idxs.size() << std::endl;
//        throw std::runtime_error("NICE");
    }

    std::size_t r_table_size() const {
        return n_*n_-1;
    }

    std::size_t delta_table_size() const {
        return n_*n_*(max_precondition_size_+1+1);
    }

    //! Return the "achiever satisfaction factor" #q(s) for the given state s and atom q,
    //! which is the min k such that there is a ground action that achieves q and has
    //! k unsatisfied preconditions in state s
    unsigned compute_achiever_satisfaction_factor(const State& state, unsigned var) const {
        unsigned min_unach_precs = std::numeric_limits<unsigned>::max();
        for (const auto& actionidx:achievers_[var]) {
            unsigned unachieved = 0;
            for (const auto& pre:operators_[actionidx].precondition_) {
                if (state.getValue(pre.first) != pre.second) {
                    unachieved++;
                }
            }
            min_unach_precs = std::min(min_unach_precs, unachieved);
        }
//        std::cout << "k=" << min_unach_precs << "; " << state << std::endl;

        return min_unach_precs;
    }

    std::vector<bool> reached_atoms() const override {
        return reached_;
    }

    void reset() override {
//        seen_.swap(std::vector<bool>(delta_table_size(), false));
//        reached_.swap(std::vector<bool>(nvars_, false));
        reached_.clear();
        seen_.clear();
        r_tables_.clear();
    }

    unsigned evaluate(NodeT& node) override {
        const State& state = node.state;

        const auto& valuation = get_valuation(this->_featureset.evaluate(state));
        assert(state.numAtoms() == nvars_);
        assert(valuation.size() == nvars_);

        bool is_novel = false;

        unsigned action_id = get_action_id(node.action);
        bool is_root = (action_id == std::numeric_limits<unsigned>::max());
//        is_root = true;

        unsigned num_atoms_true = 0;
        for (unsigned q = 0; q < nvars_; ++q) {
            if (valuation[q]) num_atoms_true++;
        }


        // Check context #r
        if (!is_root) {
            for (const auto& p:op_adds_[action_id]) {
                if (process_p_for_context_r(valuation, p, num_atoms_true)) {
                    is_novel = true;
                }
            }
        } else {
            for (unsigned p = 0; p < nvars_; ++p) {
                if (process_p_for_context_r(valuation, p, num_atoms_true)) {
                    is_novel = true;
                }
            }
        }

        if (is_novel) return 1;

        // Check <q, delta(q)> contexts
        for (unsigned q = 0; q < nvars_; ++q) {
            auto qval = make_object(valuation[q]);
            if (!atom_idx_.is_indexed(q, qval)) continue;
            unsigned qidx = atom_idx_.to_index(q, qval);

            unsigned k = 0;
            if (reached_[qidx]) {
                // We just consider a new context where q is reached
                k = max_precondition_size_+1;

            } else {
                k = this->compute_achiever_satisfaction_factor(state, q);
//            if (k == std::numeric_limits<unsigned>::max()) continue;  // Atom q has no possible achiever.
                if (k == std::numeric_limits<unsigned>::max()) k = 0;
            }


            if (!is_root) {
                for (const auto& p:op_adds_[action_id]) {
                    if (process_p(valuation, k, qidx, p)) {
                        is_novel = true;
                        if (this->config_.break_on_first_novel_) return 1;
                    }
                }
            } else {
                for (VariableIdx p = 0; p < nvars_; ++p) {
                    if (process_p(valuation, k, qidx, p)) {
                        is_novel = true;
                        if (this->config_.break_on_first_novel_) return 1;
                    }
                }
            }
        }

        return is_novel ? 1 : std::numeric_limits<unsigned>::max();
    }


    bool process_p(const std::vector<bool>& valuation, unsigned k, unsigned qidx, unsigned p) {
        auto pval = make_object(valuation[p]);
        if (!atom_idx_.is_indexed(p, pval)) return false;
        unsigned pidx = atom_idx_.to_index(p, pval);

        reached_[pidx] = true;

        unsigned atom_index = _combine_indexes(k, qidx, pidx, this->atom_idx_.size());
        assert(atom_index < seen_.size());
        auto&& ref = seen_[atom_index];
        if (!ref) { // The tuple is new
            ref = true;
            return true;
        }

        return false;
    }

    bool process_p_for_context_r(const std::vector<bool>& valuation, VariableIdx p, unsigned num_atoms_true) {
        auto pval = make_object(valuation[p]);
        if (!atom_idx_.is_indexed(p, pval)) return false;
        unsigned pidx = atom_idx_.to_index(p, pval);

        reached_[pidx] = true;

        auto rindex = _combine_rcontext_indexes(pidx, num_atoms_true, n_);
        assert(rindex < r_tables_.size());
        auto&& ref = r_tables_[rindex];
        if (!ref) { // The tuple is new
            ref = true;
            return true;
        }

        return false;
    }


protected:
    const AtomIndex& atom_idx_;

    unsigned n_;

    const FeatureSetT& _featureset;

    std::vector<SASPlusOperator> operators_;

    std::vector<std::vector<unsigned>> op_adds_;

    std::vector<std::vector<unsigned>> achievers_;

    const AchieverNoveltyConfiguration& config_;
    unsigned max_precondition_size_;
    unsigned nvars_;
    std::vector<bool> reached_;
    std::vector<bool> seen_;
    std::vector<bool> r_tables_;
};

//! Factory method: create an specialized achiever-evaluator based on the potential
//! size of the novelty tables
template<
        typename NodeT,
        typename FeatureSetT,
        typename NoveltyEvaluatorT
>
std::unique_ptr<BitvectorAchieverNoveltyEvaluator<NodeT, FeatureSetT, NoveltyEvaluatorT>>
create_achiever_evaluator(const Problem& problem,
        const FeatureSetT& features,
        const std::vector<SASPlusOperator>& operators,
        const AchieverNoveltyConfiguration& config) {

    const auto& atom_idx = problem.get_tuple_index();
    unsigned nvars = ProblemInfo::getInstance().getNumVariables();
    std::vector<std::vector<unsigned>> achievers(nvars);

    // A vector with one element X per operator o; where X is a vector with all variables that _might_
    // become true on application of o. "Might" stands for the possibility of conditional effects
    std::vector<std::vector<unsigned>> op_adds(operators.size());

    std::size_t max_precondition_size = 0;
    for (std::size_t actionidx = 0, n = operators.size(); actionidx < n; ++actionidx) {
        const auto& op = operators[actionidx];

        max_precondition_size = std::max(max_precondition_size, op.precondition_.size());

        for (const auto& eff:op.effects_) {
            if (eff.second == object_id::TRUE) {
                achievers[eff.first].push_back(actionidx);
                op_adds[actionidx].push_back(eff.first);
            }
        }
    }

    unsigned long expected_table_entries = nvars*nvars*(max_precondition_size+1);
    unsigned long expected_delta_table_size_in_kb = expected_table_entries / (8 * 1024); // size in kilobytes

    LPT_INFO("cout", "Max. precondition size: " << max_precondition_size);
    LPT_INFO("cout", "Num. state variables: " << nvars);
    LPT_INFO("cout", "Expected table size: " << expected_delta_table_size_in_kb << "KB (entries: " << expected_table_entries << ", max. size: " << config.max_table_size_ <<")");

    using ET = BitvectorAchieverNoveltyEvaluator<NodeT, FeatureSetT, NoveltyEvaluatorT>;
    return std::make_unique<ET>(atom_idx, features, operators, op_adds, achievers, max_precondition_size, nvars, config);
}

} // namespaces