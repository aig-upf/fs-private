"""
    A module to deal with the instantiation of state variables.
"""

import itertools

from .fstrips import Variable
from .util import is_external, IndexDictionary


def create_all_possible_state_variables(symbols, static_symbols, type_map):
    """ Creates an index with all possible state variables """
    variables = IndexDictionary()

    for symbol in symbols.values():
        name = symbol.name
        if is_external(name) or name in static_symbols:  # The symbol won't yield any state variable
            continue
        try:
            instantiations = [type_map[t] for t in symbol.arguments]
        except TypeError as e:
            print(type_map)
            print(symbol.arguments)
            raise RuntimeError("Error processing symbol '{}' arguments: {}".format(symbol.name, symbol.arguments))
        for instantiation in itertools.product(*instantiations):
            variables.add(Variable(symbol.name, instantiation))
    return variables


def create_all_possible_state_variables_from_groundings(predicates, functions, object_idx, static_symbols):
    variables = IndexDictionary()

    for pred in predicates:
        name = pred.name
        if is_external(name) or name in static_symbols:
            continue

        grounding_ids = []  # A list with the (integer index of) each grounding
        for grounding in pred.groundings:
            grounding_ids.append(tuple(object_idx.get_index(obj_name) for obj_name in grounding))

        for grounding in sorted(grounding_ids):  # IMPORTANT to output the groundings in lexicographical order
            variables.add(Variable(name, [object_idx.get_object(obj_id) for obj_id in grounding]))

    if functions:
        raise RuntimeError("Gringo grounder not yet prepared for functions")

    return variables
