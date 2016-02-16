"""
 Some commonly-used helpers
"""
from pddl import Type
from pddl.pddl_types import set_supertypes, TypedObject
from taskgen import process_type_hierarchy
from index import CompilationIndex


def process_types(types):
    """
    Process a set of directly-specified types.
    :param types: The types in a straight-forward {'typename': 'parent_typename'} dictionary format
    :return: The types as processed by our preprocessor from the corresponding FD PDDL parser format
    """
    processed = [Type(name, parent) for name, parent in types.items()]
    set_supertypes(processed)
    return process_type_hierarchy(processed)


def generate_objects(objects):
    """
    Process a set of directly-specified objects.
    :param objects: The objects in a straight-forward {'objectname': 'typename'} dictionary format
    :return: The objects in FD's PDDL parser format
    """
    return [TypedObject(name, type_) for name, type_ in objects.items()]


class MockTask(object):
    """
    A base, mock task that can contain a number, but possibly not all, of the necessary task components.
    """
    def __init__(self, **kwargs):
        defaults = dict(types=[], supertypes=[], objects=[], static_symbols=[], fluent_symbols=[])
        for name, default in defaults.items():
            object.__setattr__(self, name, kwargs[name] if name in kwargs else default)
        self.all_symbols = self.static_symbols + self.fluent_symbols
        self.index = CompilationIndex(self.objects, self.types)
