
#pragma once

#include <vector>
#include <memory>

template <typename NodeT>
class SimulationEvaluatorI {
public:
    virtual unsigned evaluate(NodeT& node) = 0;
    virtual void reset() = 0;
    virtual std::vector<bool> reached_atoms() const = 0;
};

template <typename NodeT, typename FeatureSetT, typename NoveltyEvaluatorT>
class SimulationEvaluator : public SimulationEvaluatorI<NodeT> {
protected:
    //! The set of features used to compute the novelty
    const FeatureSetT& _features;

    //! A single novelty evaluator will be in charge of evaluating all nodes
    std::unique_ptr<NoveltyEvaluatorT> _evaluator;

public:
    SimulationEvaluator(const FeatureSetT& features, NoveltyEvaluatorT* evaluator) :
            _features(features),
            _evaluator(evaluator)
    {}

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
