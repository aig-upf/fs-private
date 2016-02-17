"""
 Some commonly-used helpers
"""
from pddl import Type, Predicate, Action, Conjunction
from pddl.functions import TypedFunction
from pddl.pddl_types import set_supertypes, TypedObject
from object_types import process_problem_types
from pddl.tasks import DomainBound


def process_types(types, fd_objects):
    return process_problem_types(generate_fd_types(types), fd_objects, [])


def generate_fd_types(types):
    fd_types = [Type(name, parent) for name, parent in types]
    set_supertypes(fd_types)
    return fd_types


def generate_fd_bound(bound):
    typename, min_, max_ = bound
    return DomainBound(typename, "int[{}..{}]".format(min_, max_))


def generate_fd_function(data):
    name, args, type_ = data
    args = generate_fd_objects(args)
    return TypedFunction(name, args, type_)


def generate_fd_predicate(data):
    name, args = data
    args = generate_fd_objects(args)
    return Predicate(name, args)


def generate_fd_action(data):
    name, parameters, precondition, effects = data
    precondition = Conjunction(precondition)
    return Action(name, parameters, 0, precondition, effects, 0)


def generate_fd_dummy_action():
    return generate_fd_action(('dummy_action', [], [], []))


def generate_fd_objects(objects):
    """
    Process a set of directly-specified objects.
    :param objects: The objects in a straight-forward {'objectname': 'typename'} dictionary format
    :return: The objects in FD's PDDL parser format
    """
    return [TypedObject(name, type_) for name, type_ in objects.items()]
