
import util


class Atom(object):
    """ A generic (possibly functional) fact """
    def __init__(self, var, value):  # TODO - Maybe use indexes directly?
        self.var = var
        self.value = value

    def normalize(self):
        return self

    def is_predicate(self):
        return isinstance(self.value, bool)

    def is_equality(self):
        return self.symbol == '='

    def __hash__(self):
        return hash((self.var, self.value))

    def __eq__(self, other):
        return (self.var, self.value) == (other.var, other.value)

    def ground(self, binding):
        self.var.ground(binding)
        if util.is_action_parameter(self.value):
            self.value = binding[self.value]
        return self

    def __str__(self):
        return '{} {} {}'.format(self.var, self.OP, self.value)
    __repr__ = __str__
    OP = '='

    @property
    def symbol(self):
        return self.var.symbol

    @property
    def args(self):
        return self.var.args



class IndexDictionary(object):
    """
    A very basic indexing mechanism object that assigns consecutive indexes to the indexed objects.
    """
    def __init__(self):
        self.obj_to_idx = {}
        self.idx_to_obj = []

    def __getitem__(self, key):
        return self.obj_to_idx[key]

    def index(self, obj):
        if obj in self.obj_to_idx:
            raise RuntimeError("Duplicate object '{}'".format(obj))
        self.obj_to_idx[obj] = len(self.idx_to_obj)
        self.idx_to_obj.append(obj)

    def get_index(self, obj):
        return self.obj_to_idx[obj]

    def dump_index(self, print_index=True):
        def printer(i, o):
            return "{}.{}".format(i, o) if print_index else str(o)
        return [printer(i, o) for i, o in enumerate(self.idx_to_obj)]

    def __len__(self):
        return len(self.idx_to_obj)


def bool_string(value):
    return '_true_' if value else '_false_'