"""
Several methods for generating tasks.
"""
from collections import deque, defaultdict
import re
import base
from base import ProblemDomain, ProblemInstance
from compilation.exceptions import TypeException
from compilation.schemata import ActionSchemaProcessor
from index import CompilationIndex, index_symbols


def process_action_schemata(task):
    return [ActionSchemaProcessor(task, action).process() for action in task.actions]


def create_problem_domain(task, domain_name):
    """ Create a problem domain and perform the appropriate validity checks """
    types, supertypes = process_type_hierarchy(task.types)
    symbols, symbol_types = index_symbols(task)
    task.index = CompilationIndex(task.objects, types)
    schemata = process_action_schemata(task)
    return ProblemDomain(domain_name, types, supertypes, symbols, symbol_types, schemata)


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


def create_problem_instance(name, domain, translator):
    # TODO - This is extremely messy - CLEAN UP, PLEASE!
    objects = translator.get_objects()
    init = translator.process_initial_state(domain.symbols)
    data = translator.get_static_data()
    task = translator.task
    instance = ProblemInstance(name, domain, objects, init, data)

    instance.type_map, instance.object_type = process_types(instance, task.bounds)

    # For easier reference
    instance.types = instance.domain.types
    instance.symbols = instance.domain.symbols
    instance.static_symbols = task.static_symbols
    instance.fluent_symbols = task.fluent_symbols
    instance.task = task
    instance.goal_formula = translator.get_goal_formula()
    instance.state_constraints = translator.get_state_constraints()

    return instance


def process_bounds(bounds):

    bounded_types = {}
    for bound in bounds:
        res = re.match('int\[(.*)\.\.(.*)\]', bound.bound)
        assert res is not None
        lower = int(res.group(1))
        upper = int(res.group(2))
        if lower > upper:
            raise RuntimeError("Incorrect bound {}".format(bound))

        # bounded_types[bound.typename] = (lower, upper)
        bounded_types[bound.typename] = list(range(lower, upper + 1))
    return bounded_types


def process_types(instance, bounds):
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

    bounded_types = process_bounds(bounds)

    # Always add the bool, object and number types
    type_map['_bool_'] = ['_false_', '_true_']
    type_map['object'] = []
    type_map['number'] = []

    # for every type we append the corresponding object
    for o in instance.objects:
        object_type[o.name] = o.typename
        type_map[o.typename].append(o.name)  # Index by the object type

        # Then index by all of the parent types
        if o.typename != 'object':  # Sometimes type 'object' is parsed as having supertype 'object' as well.
            if o.typename not in instance.domain.supertypes:
                raise ValueError("Unkown type '{}'".format(o.typename))
            for t in instance.domain.supertypes[o.typename]:
                type_map[t].append(o.name)

    type_map = dict(list(type_map.items()) + list(bounded_types.items()))  # merge the two dictionaries

    return type_map, object_type


def process_type_hierarchy(task_types):
    """
    Return a map mapping each type name to all of its parents.
    :param task_types: The list of task types as returned by the FD PDDL parser
    """
    types = []
    all_t = set()
    correctly_declared = set()

    # The base 'object' and '_bool_' type are always there.
    # Warning: the position in the list of types is important.
    types.append(base.ObjectType('object', None))
    types.append(base.ObjectType('_bool_', 'object'))

    for t in task_types:
        if t.name != 'object':
            all_t.update([t.name, t.basetype_name])
            correctly_declared.add(t.name)
            types.append(base.ObjectType(t.name, t.basetype_name))

    # Add missing types: Some types declared without trailing " - object" sometimes are not recognized by the FD parser
    types += (base.ObjectType(t, "object") for t in all_t.difference(correctly_declared, ['object']))

    # Build a temporary map from any type to all its direct children types.
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

    return types, supertypes
