from collections import OrderedDict

import util
import base
from compilation.helper import get_effect_symbol


class IndexDictionary(object):
    """
    A very basic indexing mechanism object that assigns consecutive indexes to the indexed objects.
    """
    def __init__(self):
        self.data = OrderedDict()

    def get_index(self, key):
        return self.data[key]

    def add(self, obj):
        if obj in self.data:
            raise RuntimeError("Duplicate element '{}'".format(obj))
        self.data[obj] = len(self.data)

    def dump(self):
        return [str(o) for o in self.data.keys()]

    def __str__(self):
        return ','.join('{}: {}'.format(k, o) for k, o in self.data.items())

    __repr__ = __str__

    def __iter__(self):
        return self.data.__iter__()

    def __contains__(self, k):
        return k in self.data

    def __len__(self):
        return len(self.data)


class CompilationIndex(object):
    def __init__(self, objects, types):
        self.objects = self.index_objects(objects)  # Each object name points to it unique 0-based index / ID
        self.types = {t.name: i for i, t in enumerate(types)}  # Each typename points to its (unique) 0-based index

    @staticmethod
    def index_objects(objects):
        idx = IndexDictionary()
        idx.add(util.bool_string(False))  # 0
        idx.add(util.bool_string(True))  # 1
        # idx.add('undefined')  # Do we need an undefined object?
        for o in objects:
            idx.add(o.name)
        return idx


def index_symbols(task):
    """
     This method takes care of analyzing any given task to determine which of the task symbols
     are fluent and which static.
    """
    symbols, types = [], {}

    for s in task.predicates:
        argtypes = [t.type for t in s.arguments]
        symbols.append(base.Predicate(s.name, argtypes))
        types[s.name] = '_bool_'

    for s in task.functions:
        if s.name != 'total-cost':  # Ignore the "fake" total-cost function
            argtypes = [t.type for t in s.arguments]
            symbols.append(base.Function(s.name, argtypes, s.type))
            types[s.name] = s.type

    task.all_symbols = list(types.keys())

    # All symbols appearing on some action effect are fluent
    task.fluent_symbols = set(get_effect_symbol(eff) for action in task.actions for eff in action.effects)

    # The rest are static, including, by definition, the equality predicate
    task.static_symbols = set(s for s in task.all_symbols if s not in task.fluent_symbols) | set("=")

    return symbols, types