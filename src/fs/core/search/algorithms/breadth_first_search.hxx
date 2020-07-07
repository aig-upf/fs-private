/*
Lightweight Automated Planning Toolkit (LAPKT)
Copyright (C) 2015

<contributors>
Miquel Ramirez <miquel.ramirez@gmail.com>
</contributors>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <lapkt/algorithms/generic_search.hxx>
#include <lapkt/search/components/open_lists.hxx>
#include <lapkt/search/components/stl_unordered_map_closed_list.hxx>
#include <fs/core/problem_info.hxx>
#include <fs/core/problem.hxx>
#include <fs/core/actions/action_id.hxx>

namespace lapkt { namespace blai {


template <typename ActionIDT>
std::string print_action_schema_name(const fs0::Problem& problem, const ActionIDT& action_id) {
    throw std::runtime_error("Not implemented");
    return "not-implemented";
}


std::string print_action_schema_name(const fs0::Problem& problem, unsigned action_id) {
    if (action_id > problem.getGroundActions().size()) {
        return "no-action";
    }
    return problem.getGroundActions().at(action_id)->getName();
}

//! Partial specialization of the GenericSearch algorithm:
//! A breadth-first search is a generic search with a FIFO open list and
//! a standard unsorted closed list. Type of node and state model are still generic.
template<typename NodeT,
        typename StateModel,
        typename OpenListT = lapkt::SearchableQueue<NodeT>,
        typename ClosedListT = aptk::StlUnorderedMapClosedList<NodeT>
>
class StlBreadthFirstSearch : public GenericSearch<NodeT, OpenListT, ClosedListT, StateModel> {
public:
    using BaseClass = GenericSearch<NodeT, OpenListT, ClosedListT, StateModel>;
    using StateT = typename BaseClass::StateT;
    using PlanT = typename BaseClass::PlanT;
    using NodePT = typename BaseClass::NodePT;

    using NodeOpenEvent = typename BaseClass::NodeOpenEvent;
    using GoalFoundEvent = typename BaseClass::GoalFoundEvent;
    using NodeExpansionEvent = typename BaseClass::NodeExpansionEvent;
    using NodeCreationEvent = typename BaseClass::NodeCreationEvent;

    using ActionId = typename StateModel::ActionId;

    //! The constructor requires the user of the algorithm to inject both
    //! (1) the state model to be used in the search
    //! (2) the particular open and closed list objects
    StlBreadthFirstSearch(const StateModel& model, OpenListT&& open, long max_expansions, bool stop_on_goal) :
            BaseClass(model, std::move(open), ClosedListT()),
            _max_expansions(max_expansions),
            _stop_on_goal(stop_on_goal)
    {}

    virtual ~StlBreadthFirstSearch() = default;

    // Disallow copy, but allow move_max_expansions
    StlBreadthFirstSearch(const StlBreadthFirstSearch&) = delete;

    StlBreadthFirstSearch(StlBreadthFirstSearch&&) = default;

    StlBreadthFirstSearch& operator=(const StlBreadthFirstSearch&) = delete;

    StlBreadthFirstSearch& operator=(StlBreadthFirstSearch&&) = default;

    void log_generated_node(NodeT& n, bool goal, bool deadend) {
        const auto& info = fs0::ProblemInfo::getInstance();
//        std::string schema = print_action_schema_name(fs0::Problem::getInstance(), n.action);

        std::cout << "(N) " << n._gen_order << " " << goal << " " << deadend << " ";
        // THIS IS COPY-PASTED FROM THE STATE PRINTER
        const fs0::State& s = n.state;
        bool something_already_printed = false;
        for (unsigned x = 0; x < info.getNumVariables(); ++x) {
            fs0::object_id o = s.getValue(x);
            std::string atom;

            if (fs0::o_type(o) == fs0::type_id::bool_t) {
                if (fs0::value<bool>(o)) {
                    atom = info.getVariableName(x); // print positive atoms only
                }
            } else {
                atom = info.getVariableName(x) + "=";
                if (fs0::o_type(o) == fs0::type_id::invalid_t) atom += "<invalid>";
                else atom += info.object_name(o);
            }

            if (!atom.empty()) {
                if (something_already_printed) std::cout << " ";
                std::cout << atom;
                something_already_printed = true;
            }
        }
        std::cout << std::endl;
    }

    void log_edge(unsigned parent_id, unsigned child_id) {
        std::cout << "(E) " << parent_id << " " << child_id << std::endl;
    }

    bool is_deadend(const StateT& s) const {
        auto app = this->_model.applicable_actions(s);
        return app.begin() == app.end();
    }


    //! We redefine where the whole search schema following Russell&Norvig.
    //! The only modification is that the check for whether a state is a goal
    //! or not is done right after the creation of the state, instead of upon expansion.
    //! On a problem that has a solution at depth 'd', this avoids the worst-case expansion
    //! of all the $b^d$ nodes of the last (deepest) layer (where b is the branching factor).
    bool search(const StateT& s, PlanT& solution) override {
        NodePT n = std::make_shared<NodeT>(s, this->_generated++);

        int expanded = 0;
        std::cout << "Exploring state space. stop_on_goal=" << _stop_on_goal << ", max_expansions=" << _max_expansions
                  << std::endl;
        std::cout  << std::endl  << std::endl << "== GENERATED NODES ==" << std::endl;
        bool root_is_goal = this->check_goal(n, solution);
        log_generated_node(*n, root_is_goal, is_deadend(s));
        if (_stop_on_goal && root_is_goal) return true;

        this->_open.insert(n);

        while (!this->_open.empty() && (_max_expansions < 0 || expanded < _max_expansions)) {
            NodePT current = this->_open.next();

            // close the node before the actual expansion so that children which are identical
            // to 'current' get properly discarded
            this->_closed.put(current);
            ++expanded;

            for (const auto& a : this->_model.applicable_actions(current->state)) {
                StateT s_a = this->_model.next(current->state, a);
                NodePT successor = std::make_shared<NodeT>(std::move(s_a), a, current, this->_generated++);

                auto repeated = this->_closed.seek(successor);
                if (repeated != nullptr) { // The node has already been closed
                    log_edge(current->_gen_order, repeated->_gen_order);
                    this->_generated--; // And reset the ID to the previous state
                    continue;
                }

                repeated = this->_open.seek(successor);
                if (repeated != nullptr) { //   The node is already in the open list
                    log_edge(current->_gen_order, repeated->_gen_order);
                    this->_generated--; // And reset the ID to the previous state
                    continue;
                }

                // Otherwise, we have a new node:
                bool is_goal = this->check_goal(successor, solution);
                bool deadend = is_deadend(successor->state);
                log_generated_node(*successor, is_goal, deadend);
                log_edge(current->_gen_order, successor->_gen_order);

                if (_stop_on_goal && is_goal) return true;

                bool inserted = this->_open.insert(successor);
                if (!inserted) { // If we didn't insert the node because of novelty considerations, we push it into the
                                 // closed list so that it'll be detected as duplicate
                    this->_closed.put(successor);
                }
            }
        }

        std::cout  << std::endl << "== ALL STATE SPACE EXPANDED ==" << std::endl << std::endl;
        return false;
    }

protected:
    long _max_expansions;
    bool _stop_on_goal;
};

}}
