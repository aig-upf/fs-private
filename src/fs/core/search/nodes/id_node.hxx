//
// Created by Miquel Ramirez on 11/12/2019.
//

#pragma once

#include <lapkt/tools/logging.hxx>

namespace lapkt {

    template <typename StateT, typename ActionT, typename ActionIterator>
    class IterativeDeepeningNode {
    public:
        StateT state; // TODO - Check no extra copies are being performed, or switch to pointers otherwise.

        using ActionIdT = typename ActionT::IdType;
        ActionIdT action;

        IterativeDeepeningNode<StateT, ActionT, ActionIterator>* parent{nullptr};

        unsigned g{0};

        ActionIterator                      app;
        typename ActionIterator::Iterator   act_iter;

    public:
        IterativeDeepeningNode() = default;
        ~IterativeDeepeningNode() = default;

        IterativeDeepeningNode(const IterativeDeepeningNode&) = delete;
        IterativeDeepeningNode(IterativeDeepeningNode&&) = delete;
        IterativeDeepeningNode& operator=(const IterativeDeepeningNode&) = delete;
        IterativeDeepeningNode& operator=(IterativeDeepeningNode&&) = delete;

        //! Constructor with full copying of the state (expensive)
        explicit IterativeDeepeningNode( const StateT& s, unsigned long gen_order = 0)
                : state( s ), action( ActionT::invalid_action_id ), parent( nullptr ), g(0)
        {}

        //! Constructor with move of the state (cheaper)
        IterativeDeepeningNode( StateT&& _state, ActionIdT _action, IterativeDeepeningNode<StateT, ActionT, ActionIterator>* _parent, unsigned long gen_order = 0) :
                state(std::move(_state)), action(_action), parent(_parent), g(parent->g+1)
        {}

        bool has_parent() const { return parent != nullptr; }
        bool more_actions() const { return act_iter != app.end(); }

        //! Print the node into the given stream
        friend std::ostream& operator<<(std::ostream &os, const IterativeDeepeningNode<StateT, ActionT, ActionIterator>& object) { return object.print(os); }
        std::ostream& print(std::ostream& os) const {
            os << "{@ = " << this << ", s = " << state << ", parent = " << parent << "}";
            return os;
        }

        bool operator==( const IterativeDeepeningNode<StateT, ActionT, ActionIterator>& o ) const { return state == o.state; }

        std::size_t hash() const { return state.hash(); }
    };

}  // namespaces
