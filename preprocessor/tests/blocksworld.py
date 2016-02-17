"""
    Some generators to generate components from the standard blocksworld domain.
"""
from pddl import Atom, NegatedAtom
from .common import generate_fd_predicate, generate_fd_action, generate_fd_objects, generate_fd_effect


def generate_fd_bw_predicates():
    on = generate_fd_predicate(('on', [('?b1', 'block'), ('?b2', 'block')]))
    clear = generate_fd_predicate(('clear', [('?b', 'block')]))
    ontable = generate_fd_predicate(('ontable', [('?b', 'block')]))
    h_empty = generate_fd_predicate(('handempty', []))
    return [clear, h_empty, on, ontable]


def generate_fd_bw_actions():
    # name, parameters, precondition, effects = data

    precs = [Atom('clear', ['?x']), Atom('ontable', ['?x']), Atom('handempty', [])]
    effects = [generate_fd_effect(NegatedAtom('ontable', ['?x'])),
               generate_fd_effect(NegatedAtom('clear', ['?x'])),
               generate_fd_effect(NegatedAtom('handempty', [])),
               generate_fd_effect(Atom('holding', ['?x']))]

    pu = generate_fd_action(('pick-up', generate_fd_objects([('?x', 'block')]), precs, effects))

    precs = [Atom('holding', ['?x'])]
    effects = [generate_fd_effect(Atom('ontable', ['?x'])),
               generate_fd_effect(Atom('clear', ['?x'])),
               generate_fd_effect(Atom('handempty', [])),
               generate_fd_effect(NegatedAtom('holding', ['?x']))]

    pd = generate_fd_action(('put-down', generate_fd_objects([('?x', 'block')]), precs, effects))

    precs = [Atom('clear', ['?y']), Atom('holding', ['?x'])]
    effects = [
        generate_fd_effect(NegatedAtom('holding', ['?x'])),
        generate_fd_effect(NegatedAtom('clear', ['?y'])),
        generate_fd_effect(Atom('clear', ['?x'])),
        generate_fd_effect(Atom('handempty', [])),
        generate_fd_effect(Atom('on', ['?x', '?y']))
    ]

    st = generate_fd_action(('stack', generate_fd_objects([('?x', 'block'), ('?y', 'block')]), precs, effects))

    precs = [Atom('clear', ['?x']), Atom('on', ['?x', '?y']), Atom('handempty', [])]
    effects = [
        generate_fd_effect(Atom('holding', ['?x'])),
        generate_fd_effect(Atom('clear', ['?y'])),
        generate_fd_effect(NegatedAtom('clear', ['?x'])),
        generate_fd_effect(NegatedAtom('handempty', [])),
        generate_fd_effect(NegatedAtom('on', ['?x', '?y']))
    ]
    us = generate_fd_action(('unstack', generate_fd_objects([('?x', 'block'), ('?y', 'block')]), precs, effects))

    return [pu, pd, st, us]
