"""
Several methods for generating VPM tasks.
"""
from collections import deque, defaultdict
import operator
from functools import reduce

import base
from base import ProblemDomain, ProblemInstance, Function, Predicate
from compilation.exceptions import TypeException


def process_symbol_types(symbols):
    types = {}
    for symbol in symbols:
        if isinstance(symbol, Function):
            types[symbol.name] = symbol.codomain
        elif isinstance(symbol, Predicate):
            types[symbol.name] = '_bool_'
        else:
            raise RuntimeError("Unkown symbol type")

    return types


def create_problem_domain(name, types, symbols, actions):
    """ Create a problem domain and perform the appropriate validity checks """
    domain = ProblemDomain(name, types, symbols, actions)
    domain.supertypes = process_type_hierarchy(types)
    domain.symbol_types = process_symbol_types(symbols)
    return domain


def check_type(instance, expected, value):
    """
    Checks that the `value` object has a type compatible with the `expected` type, taking into account _int_ values
    and the domain type hierarchy.
    """
    # Special check for the integer type
    if isinstance(value, int):
        if expected != "_int_":
            raise TypeException("Wrong object type 'int' for object '{}'.Expected type '{}'".format(value, expected))
        return

    # Now the regular check taking into account the type hierarchies
    immediate_type = instance.object_type[value]
    if immediate_type != expected and not expected in instance.domain.supertypes[immediate_type]:
        raise TypeException("Wrong object type '{}' for object '{}'.Expected type '{}'"
                            .format(immediate_type, value, expected))


def check_state_complete(instance, state):
    """ Check the state is complete, i.e. that has a properly defined value for all possible state variables. """
    domain = instance.domain
    for instantiation in state.instantiations.values():
        assert isinstance(instantiation, (base.PredicateInstantiation, base.FunctionInstantiation))
        if not instantiation.symbol in domain.symbols:
            raise Exception("Unkown symbol '{}'".format(instantiation.symbol))
        symbol = domain.symbols[instantiation.symbol]
        argument_types = symbol.arguments  # The types of the arguments of the symbol
        argument_arities = (len(instance.type_map[argtype]) for argtype in argument_types)
        num_expected_instantiations = reduce(operator.mul, argument_arities, 1)  # The total arity of the symbol domain

        if isinstance(instantiation, base.PredicateInstantiation):
            # Check that the predicate arguments are all of the right type
            for args in instantiation.set:
                for i, arg in enumerate(args):
                    try:
                        check_type(instance, argument_types[i], arg)
                    except TypeException as e:
                        raise TypeException("Type exception  while checking positional argument #{} of symbol {}: {}"
                                            .format(i, symbol.name, str(e)))

        else:  # We have a FunctionInstantiation
            if len(instantiation.mapping) != num_expected_instantiations:
                raise Exception("Symbol '{}' is not correctly instantiated in the given state,"
                                " since the value of some domain points is missing".format(symbol.name))

            for args, val in instantiation.mapping.items():
                # Check that the function arguments are all of the right type
                for i, arg in enumerate(args):
                    try:
                        check_type(instance, argument_types[i], arg)
                    except TypeException as e:
                        raise TypeException("Type exception  while checking positional argument #{} of symbol {}: {}"
                                            .format(i, symbol.name, str(e)))

                # Check that the function codomain is of the right type
                try:
                    check_type(instance, symbol.codomain, val)
                except TypeException as e:
                    raise TypeException("Type exception while checking codomain of symbol {}: {}"
                                        .format(symbol.name, str(e)))


def create_problem_instance(name, task, domain, objects, init, goal, static_data, constraints, gconstraints):
    """

    """
    instance = ProblemInstance(name, domain, objects, init, goal, static_data, constraints, gconstraints)

    instance.type_map, instance.object_type = process_types(instance)

    # For easier reference
    instance.types = instance.domain.types
    instance.symbols = instance.domain.symbols
    instance.static_symbols = task.static_symbols
    instance.fluent_symbols = task.fluent_symbols


    # check_state_complete(instance, init)

    return instance


def process_types(instance):
    """
    Returns (1) a map from each type to its objects.
            (2) a map from each object name to its immediate object typename

    For each object we know the type. This returns a dictionary
    from each type to a set of objects (of this type). We also
    have to care about type hierarchy. An object
    of a subtype is a specialization of a specific type. We have
    to put this object into the set of the supertype, too.
    """
    type_map = defaultdict(list)
    object_type = {}

    # for every type we append the corresponding object
    for o in instance.objects:
        object_type[o.name] = o.typename
        type_map[o.typename].append(o.name)  # Index by the object type

        # Then index by all of the parent types
        if o.typename != 'object':  # Sometimes type 'object' is parsed as having supertype 'object' as well.
            if not o.typename in instance.domain.supertypes:
                raise ValueError("Unkown type '{}'".format(o.typename))
            for t in instance.domain.supertypes[o.typename]:
                type_map[t].append(o.name)

    return type_map, object_type


def process_type_hierarchy(types):
    """
    Return a map mapping each type name to all of its parents.
    """
    # Build a map from any type to all its direct children types.
    parent_to_children = defaultdict(list)
    for t in types:
        parent_to_children[t.parent].append(t.name)

    supertypes = {'object': []}
    pending = deque(['object'])
    while pending:
        current = pending.popleft()  # Invariant: supertypes[current] includes all of current's parent types
        for t in parent_to_children[current]:
            supertypes[t] = [current] + supertypes[current]
            pending.append(t)

    return supertypes

