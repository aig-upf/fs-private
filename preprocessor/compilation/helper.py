"""
 Some helper methods.
"""
import pddl
from pddl.f_expression import NumericConstant, PrimitiveNumericExpression, FunctionalTerm

from compilation.exceptions import UnimplementedFeature


def get_formula_parts(formula):
    assert (isinstance(formula, (pddl.conditions.Conjunction, pddl.conditions.Atom)))
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


def parse_value(expression):
    if isinstance(expression, PrimitiveNumericExpression):
        return expression.symbol
    elif isinstance(expression, NumericConstant):
        return int(expression.value)
    else:
        raise UnimplementedFeature("Unsupported expression type for '{}'".format(expression))


def finish_code(code):
    return "return " + code + ";"


def finish_bool_code(code):
    return finish_code("(bool) " + code)


def assignment_code(lhs, rhs):
    return "{} = {};".format(lhs, rhs)


def index_list(l):
    """
    Given a list of hashable items, returns a dictionary having
    the items as keys and their position in the list as values
    """
    return {var: idx for idx, var in enumerate(l)}


def is_int(s):
    try:
        x = int(s)
        return str(x) == s
    except ValueError:
        return False


def is_external(symbol):
    return symbol[0] == '@'