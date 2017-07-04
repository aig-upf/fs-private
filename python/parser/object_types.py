"""
    Methods to deal with PDDL object types.
"""
import re
from collections import defaultdict, deque


def process_problem_types(fd_types, objects, fd_bounds):
    types, supertypes = process_type_hierarchy(fd_types)
    type_map = process_types(objects, supertypes, fd_bounds)
    return types, type_map, supertypes


def process_types(objects, supertypes, fd_bounds):
    """
    Returns (1) a map from each type to its objects.
            (2) a map from each object name to its immediate object typename

    For each object we know the type. This returns a dictionary
    from each type to a set of objects (of this type). We also
    have to care about type hierarchy. An object
    of a subtype is a specialization of a specific type. We have
    to put this object into the set of the supertype, too.
    """
    type_map = {k: list() for k in supertypes.keys()}

    # Always add the bool, object and int types
    type_map['object'] = []
    type_map['int'] = []
    type_map['number'] = []

    # for every type we append the corresponding object
    for o in objects:
        type_map[o.type].append(o.name)  # Index by the object type

        # Then index by all of the parent types
        if o.type != 'object':  # Sometimes type 'object' is parsed as having supertype 'object' as well.
            if o.type not in supertypes:
                raise ValueError("Unkown type '{}'".format(o.type))
            for t in supertypes[o.type]:
                type_map[t].append(o.name)

    # Add the elements corresponding to bounded types and return all types together
    bounded_types = process_bounds(fd_bounds)
    type_map.update(bounded_types)
    return type_map


def process_type_hierarchy(fd_types):
    """
    Return a map mapping each type name to all of its parents.
    :param fd_types: The list of task types as returned by the FD PDDL parser
    """
    # The base 'object' and type are always there.
    # Warning: the position in the list of types is important.
    root_types = ['object', 'int', 'number']

    types = {t: None for t in root_types}
    used_types = set()
    predeclared = set(types.keys())
    correctly_declared = set(types.keys())
    for t in fd_types:
        if t.name in predeclared:
            continue
        if t.name in correctly_declared:
            raise RuntimeError("Duplicate type declaration for type '{}'".format(t.name))
        # Add both the type and the supertype
        used_types.add(t.name)
        if t.basetype_name is not None :
            used_types.add(t.basetype_name)
        correctly_declared.add(t.name)
        types[t.name] = t.basetype_name

    # Add missing types: Some types declared without trailing " - object" sometimes are not recognized by the FD parser
    types.update({t: 'object' for t in used_types.difference(correctly_declared)})

    # Build a map from any type to all its direct children types to perform a BFS traversal.
    parent_to_children = defaultdict(list)
    for typename, parent in types.items():
        parent_to_children[parent].append(typename)

    supertypes = {t: [] for t in root_types}
    seen = set()
    pending = deque(root_types)
    while pending:
        current = pending.popleft()  # Invariant: supertypes[current] includes all of current's parent types
        seen |= {current}
        for t in parent_to_children[current]:
            supertypes[t] = [current] + supertypes[current]
            if t not in seen:
                pending.append(t)

    return types, supertypes


def process_bounds(bounds):
    bounded_types = {}
    for bound in bounds:
        res = re.match('int\[(.*)\.\.(.*)\]', bound.bound)
        assert res is not None
        lower = int(res.group(1))
        upper = int(res.group(2))
        if lower > upper:
            raise RuntimeError("Incorrect bound {}".format(bound))

        bounded_types[bound.typename] = list(range(lower, upper + 1))
    return bounded_types
