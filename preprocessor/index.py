
import util


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


class CompilationIndex(object):
    def __init__(self, task):
        self.types = {}  # Type name to type ID.
        self.objects = IndexDictionary()  # object name to object ID.
        self.typed_objects = {}
        self.symbols = {}  # symbol name to symbol ID.
        self.variables = {}  # A state variable index.

        self.index_types(task)
        self.index_objects(task)

    def index_types(self, task):
        self.types['object'] = 0
        self.types['_bool_'] = 1
        for t in task.types:
            if t.name not in ('object', '_bool_'):
                self.types[t.name] = len(self.types)

    def index_objects(self, task):
        self.objects.index(util.bool_string(False))  # 0
        self.objects.index(util.bool_string(True))  # 1
        self.objects.index('undefined')
        for o in task.objects:
            self.objects.index(o.name)
