"""
 Some helper methods related to FD's parser PDDL representation
"""
import pddl
from exceptions import UnimplementedFeature
from pddl.f_expression import FunctionalTerm
from pddl import conditions, effects


def get_formula_parts(formula):
    assert isinstance(formula, (conditions.Conjunction, conditions.Atom, conditions.NegatedAtom, conditions.Truth))
    if isinstance(formula, pddl.conditions.Truth):
        return []
    return formula.parts if isinstance(formula, pddl.conditions.Conjunction) else [formula]


def get_effect_symbol(effect):
    """ Returns the symbol affected by a singular effect. """
    if isinstance(effect.literal, (pddl.Atom, pddl.NegatedAtom)):
        return effect.literal.predicate
    elif isinstance(effect.literal, effects.AssignmentEffect):
        lhs = effect.literal.lhs
        assert isinstance(lhs, pddl.f_expression.FunctionalTerm)
        return lhs.symbol
    else:
        raise UnimplementedFeature('TODO - Effect type not yet supported')
