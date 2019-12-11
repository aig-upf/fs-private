//
// Created by Miquel Ramirez on 11/12/2019.
//

#pragma once

#include <algorithm>
#include <memory>

#include <lapkt/tools/events.hxx>

namespace lapkt {

    template <typename NodeT,
            typename StateModel,
            typename StateT = typename StateModel::StateT,
            typename ActionIdT = typename StateModel::ActionType::IdType>
    class IterativeDeepening : public events::Subject {
    private:
        // attributes
        //! The search model
        const StateModel&       _model;
        std::vector<NodeT>      _open;
        std::size_t             _generated{0};

        const std::size_t       _max_depth{10000};

    public:
        // types

        using PlanT =  std::vector<ActionIdT>;
        using NodePT = std::shared_ptr<NodeT>;

        //! Relevant events
        using NodeOpenEvent = events::NodeOpenEvent<NodeT>;
        using GoalFoundEvent = events::GoalFoundEvent<NodeT>;
        using NodeExpansionEvent = events::NodeExpansionEvent<NodeT>;
        using NodeGenerationEvent = events::NodeGenerationEvent<NodeT>;
        using NodeCreationEvent = events::NodeCreationEvent<NodeT>;

    public:
        // constructors
        explicit IterativeDeepening(const StateModel& model)
            : _model(model) {}

        ~IterativeDeepening() = default;

        // Disallow copy, allow move, and delete default constructor (we need the StateModel ref)
        IterativeDeepening() = delete;
        IterativeDeepening(const IterativeDeepening& other) = delete;
        IterativeDeepening(IterativeDeepening&& other) noexcept = default;
        IterativeDeepening& operator=(const IterativeDeepening& rhs) = delete;
        IterativeDeepening& operator=(IterativeDeepening&& rhs) noexcept = default;

    public:
        // interface

        //! Convenience method
        bool solve_model(PlanT& solution) {
            return search( _model.init(), solution );
        }

        bool
        search(const StateT& s0, PlanT& solution) {
            _open.resize(_max_depth);

            _open[0].state = std::move(StateT(s0));
            _open[0].g = 0;
            _open[0].parent = nullptr;
            this->notify(NodeCreationEvent(_open[0]));
            _open[0].app = _model.applicable_actions(_open[0].state);
            _open[0].act_iter = _open[0].app.begin();

            set_node(0, StateT(s0), 0);
            if (check_goal(_open[0], solution)) return true;
            unsigned max_g  = 0;

            for (std::size_t max_search_depth = 1; max_search_depth < _max_depth; max_search_depth++) {
                std::size_t depth = 0;
                unsigned current_max_g = 0;
                while (depth <= max_search_depth) {
                    auto& n = _open[depth];
                    if (!n.more_actions()) {
                        if (depth == 0) {
                            break;
                        }
                        depth--;
                        continue;
                    }

                    this->notify(NodeExpansionEvent(n));
                    auto a = *(n.act_iter);
                    n.act_iter++;

                    StateT s_a = _model.next(n.state, a);
                    _open[depth+1].state = std::move(s_a);
                    _open[depth+1].g = n.g + 1;
                    _open[depth+1].parent = depth == 0 ? nullptr : &_open[depth];
                    this->notify(NodeCreationEvent(_open[depth+1]));
                    _open[depth+1].app = _model.applicable_action(_open[depth+1].state);
                    _open[depth+1].act_iter = _open[depth+1].app.begin();
                    depth++;

                    if (check_goal(_open[depth], solution)) return true;
                    current_max_g = std::max(current_max_g, _open[depth].g);
                }

                if (current_max_g <= max_g) {
                    break; // Unsolvable
                }
                max_g = current_max_g;
            }

            _open.clear();
            return false;
        }

        //! Backward chaining procedure to recover a plan from a given node
        virtual void retrieve_solution(NodePT node, PlanT& solution) {
            while (node->has_parent()) {
                solution.push_back(node->action);
                node = node->parent;
            }
            std::reverse( solution.begin(), solution.end() );
        }
    protected:

        void set_node(std::size_t level, unsigned g) {

        }

        virtual bool check_goal(const NodeT& node, PlanT& solution) {
            if ( _model.goal(node.state)) { // Solution found, we're done
                this->notify(GoalFoundEvent(node));
                retrieve_solution(node, solution);
                return true;
            }
            return false;
        }

        bool check_node_correctness(const NodeT& node) const {
            if (node.has_parent()) {
                assert(_model.is_applicable(node.parent.state, node.action));
            }
            return true;
        }
    };

}