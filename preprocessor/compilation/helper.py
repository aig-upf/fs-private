"""
 Some helper methods.
"""
import pddl
from pddl.f_expression import NumericConstant, PrimitiveNumericExpression, FunctionalTerm

from compilation.exceptions import UnimplementedFeature


def get_formula_parts(formula):
    assert (isinstance(formula, (pddl.conditions.Conjunction, pddl.conditions.Atom, pddl.conditions.NegatedAtom,
                                 pddl.conditions.Truth)))
    if isinstance(formula, pddl.conditions.Truth):
        return []
    return formula.parts if isinstance(formula, pddl.conditions.Conjunction) else [formula]


def get_effect_symbol(effect):
    """ Returns the symbol affected by a singular effect. """
    if isinstance(effect.literal, (pddl.Atom, pddl.NegatedAtom)):
        return effect.literal.predicate
    elif isinstance(effect.literal, pddl.effects.AssignmentEffect):
        lhs = effect.literal.lhs
        assert(isinstance(lhs, pddl.f_expression.FunctionalTerm))
        return lhs.symbol
    else:
        raise UnimplementedFeature('TODO - Effect type not yet supported')


def is_external(symbol):
    return symbol[0] == '@'
