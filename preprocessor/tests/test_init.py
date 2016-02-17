"""
 Some tests of the handling of the initial state, static and fluent discovery, etc.
"""
import pytest

from .common import generate_base_fs_task
from .blocksworld import generate_fd_bw_predicates, generate_fd_bw_actions
from pddl.conditions import Atom


@pytest.fixture(scope="module")
def bw_task():
    """ This effectively generates a single BW instance per test module and allows it to be reused in different tests
        simply by declaring that the test has a 'bw_task' parameter. See https://pytest.org/latest/fixture.html. """
    task = generate_base_fs_task(objects=[('b1', 'block'), ('b2', 'block')], types=[('block', 'object')])
    predicates = generate_fd_bw_predicates()
    actions = generate_fd_bw_actions()
    task.process_symbols(actions, predicates, [])
    return task


def test_predicative_task(bw_task):
    init = [Atom('on', ['b2', 'b1']), Atom('clear', ['b2']), Atom('ontable', ['b1']), Atom('handempty', [])]
    bw_task.process_initial_state(init)
    initial_fluent_atoms, initial_static_data = bw_task.initial_fluent_atoms, bw_task.initial_static_data
    assert len(initial_fluent_atoms) and len(initial_static_data) == 0
