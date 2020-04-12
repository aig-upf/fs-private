"""
    This module contains a number of classes and routines to handle the static (external) data, including its
    declaration and serialization.
"""
from .. import utils


def instantiate_function(name, arity):
    if arity == 0:
        return NullaryFunction(name)
    return Map(name, arity)


def instantiate_predicate(name, arity):
    if arity == 0:
        return NullarySet(name)
    return Set(name, arity)


def serialize_symbol(symbol, table):
    if utils.is_int(symbol) or utils.is_float(symbol):
        return str(symbol)
    if symbol is None:
        return ""
    try:
        serialized = table[symbol.name]
    except AttributeError:
        serialized = table[symbol]
    return str(serialized)


def serialize_tuple(t, symbols):
    """  A small helper to serialize a whole tuple with the help of a symbol table """
    t = (t,) if not isinstance(t, (list, tuple)) else t
    return ','.join(serialize_symbol(e, symbols) for e in t)


class DataElement:
    def __init__(self, name, arity=0):
        self.name = name
        self.arity = arity

    def serialize_data(self, symbols):
        raise RuntimeError("Method must be subclassed")

    def needs_serialization(self):
        """ Whether the given data element needs serialization.
        All data elements need serialization except for those corresponding to nullary atoms that are false in the
        initial state. """
        return True


class NullarySet(DataElement):
    def __init__(self, name):
        super().__init__(name, 0)
        self.elems = {}

    def add(self, args, value=None):
        assert args == ()
        self.elems[args] = value

    def serialize_data(self, symbols):
        assert () in self.elems
        return [serialize_symbol(self.elems[()], symbols)]  # We simply print the only element

    def needs_serialization(self):
        """ Whether the given data element needs serialization.
        All data elements need serialization except for those corresponding to nullary atoms that are false in the
        initial state. """
        return () in self.elems


class NullaryFunction(DataElement):
    def __init__(self, name):
        super().__init__(name)
        self.elems = {}

    def add(self, elem, value):
        if len(elem) > 0:
            raise RuntimeError(f"Error: Arity 0 element '{self.name}' has {len(elem)} arguments: {elem}")
        assert len(elem) == 0
        self.elems[elem] = value

    def serialize_data(self, symbols):
        return [serialize_symbol(self.elems[()], symbols)]  # We simply print the only element


class Map(DataElement):
    def __init__(self, name, arity):
        super().__init__(name, arity)
        self.elems = {}

    def add(self, elem, value):
        if len(elem) != self.arity:
            raise RuntimeError("Wrong type or number of arguments for data element {}: {}({}) = {}".format(
                self.name, self.name, elem, value))
        self.elems[elem] = value

    def serialize_data(self, symbols):
        return [serialize_tuple(k + (v,), symbols) for k, v in self.elems.items()]


class Set(DataElement):
    def __init__(self, name, arity):
        super().__init__(name, arity)
        self.elems = set()

    def add(self, elem, value=None):
        assert value is None
        if len(elem) != self.arity:
            raise RuntimeError("Wrong type or number of arguments for data element {}: {}({})".format(
                self.name, self.name, elem))
        self.elems.add(elem)

    def serialize_data(self, symbols):
        return [serialize_tuple(elem, symbols) for elem in self.elems]

