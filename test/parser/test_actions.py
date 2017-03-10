"""
 Tests the correct processing of action preconditions, effects, etc.
"""
import pytest

from python.parser import exceptions
from python.parser.pddl.conditions import Atom
from .blocksworld import generate_fd_bw_predicates, generate_fd_bw_actions, generate_small_bw_instance


@pytest.fixture(scope="module")
def bw_task():
    task = generate_small_bw_instance()
    predicates = generate_fd_bw_predicates()
    actions = generate_fd_bw_actions()
    task.process_symbols(actions, predicates, [])

    init = [Atom('on', ['b2', 'b1']), Atom('clear', ['b2']), Atom('ontable', ['b1']), Atom('handempty', [])]
    task.process_initial_state(init)
    task.process_actions(actions)
    return task


def test_undeclared_symbol(bw_task):

    task = generate_small_bw_instance()
    predicates = generate_fd_bw_predicates()[1:]  # Remove one predicate !!

    actions = generate_fd_bw_actions()
    task.process_symbols(actions, predicates, [])

    with pytest.raises(exceptions.UndeclaredSymbol):
        task.process_actions(actions)


def __TODO__test_simple_action(bw_task):

    task = generate_small_bw_instance()
    predicates = generate_fd_bw_predicates()

    actions = generate_fd_bw_actions()
    task.process_symbols(actions, predicates, [])
    task.process_actions(actions)

    actions = bw_task.action_schemas

    assert 0
    # assert len(initial_fluent_atoms) and len(initial_static_data) == 0
