
#pragma once

#include <vector>
#include <memory>

#include <fs/core/search/drivers/sbfws/config.hxx>

namespace fs0::bfws {

template<typename NodeT>
class SimulationEvaluatorI {
public:
    virtual unsigned evaluate(NodeT& node) = 0;

    virtual void reset() = 0;

    virtual std::vector<bool> reached_atoms() const = 0;
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


protected:
    const FeatureSetT& _featureset;

    const NoveltyFactory<FeatureValueT> _search_novelty_factory;

//    //! A counter to count the number of unsatisfied goals
//    UnsatisfiedGoalAtomsCounter _unsat_goal_atoms_heuristic;



public:
    AchieverNoveltyEvaluator(const Problem& problem, const FeatureSetT& features) :
            _featureset(features),
            _search_novelty_factory(problem, SBFWSConfig::NoveltyEvaluatorType::Adaptive,
                    _featureset.uses_extra_features(), 1)
//            _unsat_goal_atoms_heuristic(problem)
    {
        const AtomIndex& atomidx = problem.get_tuple_index();
        for (const auto& action:problem.getGroundActions()) {
            for (const auto& eff:action->getEffects()) {
                if (!eff->is_add()) continue;
                auto sv = dynamic_cast<const fs::StateVariable*>(eff->lhs());
                assert(sv);
                sv->getValue();

            }
        }

        num_atoms_ = atomidx.size();
        for (unsigned atom = 0; atom < num_atoms_; ++atom) {


        }

    }

    ~AchieverNoveltyEvaluator() {
        for (auto& elem:tables_) delete elem.second;
    };


    unsigned evaluate(NodeT& node) override {
        unsigned min_nov = std::numeric_limits<unsigned>::max();
        for (unsigned q = 0; q < num_atoms_; ++q) {
            unsigned k = compute_achiever_satisfaction_factor(node.state, q);
            auto& table = novelty_table(q, k);
            min_nov = std::min(min_nov, table.evaluate(_featureset.evaluate(node.state), 1));
        }

        return min_nov;
    }

    //!
    unsigned compute_achiever_satisfaction_factor(const State& state, unsigned var) const {
        assert(0);

        unsigned max_precs_achieved = 0;
        for (const auto& a:achievers_[var]) {
            unsigned achieved = 0;
            for ()
            max_precs_achieved = std::max(max_precs_achieved, achieved);
        }


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
        throw std::runtime_error("Unimplemented");
    }

protected:
    unsigned num_atoms_;

    using TableKeyT = std::pair<unsigned, unsigned>;
    std::unordered_map<TableKeyT, NoveltyEvaluatorT*, boost::hash<TableKeyT>> tables_;

    std::vector<std::vector<unsigned>> achievers_;


};

} // namespaces