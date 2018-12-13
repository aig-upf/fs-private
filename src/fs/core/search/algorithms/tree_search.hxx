
#pragma once

#include <algorithm>
#include <random>
#include <memory>

#include <lapkt/tools/events.hxx>
#include <fs/core/constraints/gecode/handlers/monotonicity_csp.hxx>
#include <fs/core/utils/printers/printers.hxx>
#include <fs/core/heuristics/unsat_goal_atoms.hxx>

namespace fs0 { namespace drivers {


template <typename StateT, typename ActionT>
class TreeSearchNode {
public:
	StateT state;

	//!
	using Action = typename ActionT::IdType;
	Action action;

	//! Accumulated cost
	unsigned g;

	//! Novelty of the state
	unsigned novelty;

	//!
	bool dead;


	//!
	std::vector<std::shared_ptr<TreeSearchNode>> children;


	// Prevent copy-constructing, which is expensive because of the state and children nodes
	~TreeSearchNode() = default;
	TreeSearchNode(const TreeSearchNode& other) = delete;
	TreeSearchNode(TreeSearchNode&& other) = delete;
	TreeSearchNode& operator=(const TreeSearchNode& rhs) = delete;
	TreeSearchNode& operator=(TreeSearchNode&& rhs) = delete;

	//! Constructor with move of the state (cheaper)
	TreeSearchNode(StateT&& _state, Action a, unsigned nov) :
			state(std::move(_state)), action(a), novelty(nov), dead(false)
	{}

	//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const TreeSearchNode<StateT, ActionT>& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const {
        const Problem& info = Problem::getInstance();
        if (action != std::numeric_limits<unsigned>::max()) {
            os << "{@ = " << this << ", s = " << state << ", novelty = " << novelty << ", g = " << g << ", action: " << *(info.getGroundActions()[action]) << "}";
        } else {  // i.e. root node 
            os << "{@ = " << this << ", s = " << state << ", novelty = " << novelty << ", g = " << g << ", action: None}";
        }

		return os;
	}
};


template <typename StateModelT, typename FeatureSetT, typename NoveltyEvaluatorT>
class TreeSearch {
public:
    using StateT = typename StateModelT::StateT;
    using ActionT = typename StateModelT::ActionType;
    using ActionIdT = typename ActionT::IdType;

    using NodeT = TreeSearchNode<StateT, ActionT>;
    using NodePT = std::shared_ptr<NodeT>;

	using PlanT =  std::vector<ActionIdT>;


//	using ClosedList = aptk::StlUnorderedMapClosedList<NodeT>;



protected:
    //! The search model
    const StateModelT& _model;

    //! The closed list
//	ClosedList _closed;

    //!
    const FeatureSetT _featureset;

    //! We keep a "base" novelty evaluator with the appropriate features that we can clone before starting each trial
    std::shared_ptr<NoveltyEvaluatorT> _base_evaluator;
    std::shared_ptr<NoveltyEvaluatorT> _evaluator;

    //!
    unsigned _max_width;

    //! The number of generated nodes so far
    uint32_t _generated;


public:
    //!
    TreeSearch(const StateModelT& model, unsigned max_width, FeatureSetT&& featureset, NoveltyEvaluatorT* evaluator)
            : _model(model), _featureset(std::move(featureset)), _base_evaluator(evaluator),
            _evaluator(nullptr), _max_width(max_width), _generated(0)
    {}

	virtual ~TreeSearch() = default;
	
	// Disallow copy, but allow move
	TreeSearch(const TreeSearch& other) = delete;
	TreeSearch(TreeSearch&& other) noexcept = default;
	TreeSearch& operator=(const TreeSearch& rhs) = delete;
	TreeSearch& operator=(TreeSearch&& rhs) noexcept = default;



	unsigned selectAction(NodeT& node) {
	    LPT_DEBUG("cout", "Selecting action at inner node: " << node);
		std::vector<unsigned> candidates;
		unsigned min_novelty = std::numeric_limits<unsigned>::max();
		for (unsigned i = 0; i < node.children.size(); ++i) {
			const auto& child = *(node.children[i]);
			//
            if (child.dead) continue;
			if (child.novelty < min_novelty) {
				min_novelty = child.novelty;
				candidates.clear();
                candidates.push_back(i);

			} else if (child.novelty == min_novelty) {
				candidates.push_back(i);
			}
		}
		assert(!candidates.empty());

		// Sample a candidate uniformly at random
        // TODO Refactor this, we don't want to construct a RNG every time.
		std::vector<unsigned> out;
		std::sample(candidates.begin(), candidates.end(), std::back_inserter(out), 1, std::mt19937{std::random_device{}()});
		return out[0];
//        return candidates[0];
	}

	void backup(NodeT& node) {
		unsigned child_novelty = std::numeric_limits<int>::max();
		node.dead = true;
		for (auto& child : node.children) {
			node.dead = node.dead && child->dead;
            child_novelty = std::min(child_novelty, child->novelty);
		}
		node.novelty = std::max(child_novelty, node.novelty);
	}


	unsigned check_novelty(const StateT& state) {
        _evaluator->set_read_only(true);
	    unsigned res = _evaluator->evaluate(_featureset.evaluate(state), _max_width);
	    _evaluator->set_read_only(false);
        return res;
	}

    void update_novelty_table(const StateT& state) {
        _evaluator->evaluate(_featureset.evaluate(state), _max_width);
    }


	bool visit(NodeT& node, PlanT& solution, unsigned trial_number) {
		bool plan_found = false;

        update_novelty_table(node.state);

		if (node.children.empty()) {  // We reached a leave node

            LPT_DEBUG("cout", "Expanding: " << node);
			for (const auto& action:_model.applicable_actions(node.state)) {
				auto state = _model.next(node.state, action);

				if (_model.goal(state)) {
                    solution.push_back(action);
                    if (node.action != std::numeric_limits<unsigned>::max()) {
                        solution.push_back(node.action);
                    }
                    LPT_DEBUG("cout", state);
                    LPT_DEBUG("cout", node);

                    return true;
				}

				unsigned nov = check_novelty(state);
                node.children.push_back(std::make_shared<NodeT>(std::move(state), action, nov));

                LPT_DEBUG("cout", "\tAdding child: " << *(node.children.back()));
            }



		} else { // Inner node
			const auto& selected_child = node.children[selectAction(node)];

            plan_found = visit(*selected_child, solution, trial_number);

		}

		if (plan_found) {
		    if (node.action != std::numeric_limits<unsigned>::max()) {
                solution.push_back(node.action);
		    }
            LPT_DEBUG("cout", node);


		}
        backup(node);
		return plan_found;
	}

	virtual bool search(const StateT& s0, PlanT& plan) {


		std::shared_ptr<NodeT> root = std::make_shared<NodeT>(StateT(s0), std::numeric_limits<unsigned>::max(), 1);


		bool plan_found = false;

        unsigned trial_number = 1;
		for (; !plan_found; ++trial_number) { // While we have time, or a number of trials, etc.

//		    if (trial_number % 1000 == 0)
            LPT_INFO("cout", "\nStarting trial " << trial_number << "... ");
//		    assert(trial_number < 5);

            _evaluator = std::shared_ptr<NoveltyEvaluatorT>(_base_evaluator->clone());
			plan_found = visit(*root, plan, trial_number);

		}

        LPT_INFO("cout", "Total number of trials " << trial_number << "... ");

		std::cout << *root << std::endl;
		for (auto& child : root->children) {
            std::cout << *child << std::endl;
		}

		std::reverse(plan.begin(), plan.end());
		return plan_found;
	}

	//! Convenience method
	bool solve_model(PlanT& solution) {
        return search(_model.init(), solution);
    }
	
};

} } // namespaces
