"""
 Some helper methods related to FD's parser PDDL representation
"""
from . import pddl

from .exceptions import UnimplementedFeature
from .pddl import conditions, effects
from .pddl.f_expression import FunctionalTerm


def get_formula_parts(formula):
    assert isinstance(formula, (conditions.Conjunction, conditions.Atom, conditions.NegatedAtom, conditions.Truth))
    if isinstance(formula, conditions.Truth):
        return []
    return formula.parts if isinstance(formula, conditions.Conjunction) else [formula]


def get_effect_symbol(effect):
    """ Returns the symbol affected by a singular effect. """
    if isinstance(effect.literal, (pddl.Atom, pddl.NegatedAtom)):
        return effect.literal.predicate
    elif isinstance(effect.literal, effects.AssignmentEffect):
        lhs = effect.literal.lhs
        assert isinstance(lhs, FunctionalTerm)
        return lhs.symbol
    else:
        raise UnimplementedFeature('TODO - Effect type not yet supported, type of lhs: {}'.format(effect.literal))
