"""
 This module takes care of analyzing any given task to determine which of the task symbols
 are fluent and which static.
"""

import itertools

import pddl

from compilation.helper import get_effect_symbol, parse_value
from compilation.exceptions import ParseException


def classify_symbols(task):
    task.predicate_names = [s.name for s in task.predicates]
    task.function_names = [s.name for s in task.functions]
    task.all_symbols = list(itertools.chain(task.predicate_names, task.function_names))
    task.fluent_symbols = compute_fluent_symbols(task)
    task.static_symbols = compute_static_symbols(task, task.fluent_symbols)
    task.data = compile_init_data(task)


def compile_init_data(task):

    data = {s: dict() if s in task.function_names else set() for s in task.all_symbols}

    def check_symbol(s, point):
            if s not in data:
                raise ParseException("Unknown symbol: '{}'".format(s))
            if point in data[s]:
                raise ParseException("Duplicate initialization: '{}'".format(elem))

    for elem in task.init:
        if isinstance(elem, pddl.Assign):
            symbol = elem.fluent.symbol
            tup = elem.fluent.args
            check_symbol(symbol, tup)
            data[symbol][tup] = parse_value(elem.expression)
        elif isinstance(elem, pddl.Atom):
            symbol = elem.predicate
            tup = elem.args
            assert not elem.negated, "No negations allowed in the initialization of atoms"
            check_symbol(symbol, tup)
            data[symbol].add(tup)
        else:
            raise ParseException("Unknown initialization atom  '{}'".format(elem))

    return data


def compute_fluent_symbols(task):
    """ Return all those non-static symbols, i.e. that appear on some action effect  """
    return set(get_effect_symbol(eff) for action in task.actions for eff in action.effects)


def compute_static_symbols(task, fluent):
    """ Determine all static predicates - those which are not fluent """
    static = set(s for s in task.all_symbols if not s in fluent)
    static.add("=")  # The equality predicate is by definition static
    return static