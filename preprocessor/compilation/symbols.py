"""
 This module takes care of analyzing any given task to determine which of the task symbols
 are fluent and which static.
"""

import base
from compilation.helper import get_effect_symbol


def process_symbols(task):
    symbols, types = [], {}

    for s in task.predicates:
        argtypes = [t.type for t in s.arguments]
        symbols.append(base.Predicate(s.name, argtypes))
        types[s.name] = '_bool_'

    for s in task.functions:
        if s.name != 'total-cost':  # Ignore the fake total-cost function
            argtypes = [t.type for t in s.arguments]
            symbols.append(base.Function(s.name, argtypes, s.type))
            types[s.name] = s.type

    task.all_symbols = list(types.keys())

    # All symbols appearing on some action effect are fluent
    task.fluent_symbols = set(get_effect_symbol(eff) for action in task.actions for eff in action.effects)

    # The rest are static, including, by definition, the equality predicate
    task.static_symbols = set(s for s in task.all_symbols if s not in task.fluent_symbols) | set("=")

    return symbols, types
