"""
    This module contains a number of classes and routines to handle the static (external) data, including its
    declaration and serialization.
"""
import python.utils
from . import fstrips
from . import util


def instantiate_extension(symbol):
    if isinstance(symbol, fstrips.Predicate):
        return instantiate_predicate(symbol.name, len(symbol.arguments))
    else:  # We have a function
        return instantiate_function(symbol.name, len(symbol.arguments))


def instantiate_function(name, arity):
    classes = {0: Arity0Element, 1: UnaryMap, 2: BinaryMap, 3: Arity3Map, 4: Arity4Map}
    if arity not in classes:
        raise RuntimeError("Currently only up to arity-4 functions are supported")
    return (classes[arity])(name)


def instantiate_predicate(name, arity):
    classes = {0: ZeroarySet, 1: UnarySet, 2: BinarySet, 3: Arity3Set, 4: Arity4Set}
    if arity not in classes:
        raise RuntimeError("Currently only up to arity-4 predicates are supported")
    return (classes[arity])(name)


def serialize_symbol(symbol, table):
    if python.utils.is_int(symbol) or python.utils.is_float(symbol):
        return str(symbol)
    try :
        serialized = table[symbol.name]
    except AttributeError :
        serialized = table[symbol]
    return str(serialized)


def serialize_tuple(t, symbols):
    """  A small helper to serialize a whole tuple with the help of a symbol table """
    t = (t,) if not isinstance(t, (list, tuple)) else t
    return ','.join(serialize_symbol(e, symbols) for e in t)


class DataElement:
    def __init__(self, name):
        self.name = name

    def serialize_data(self, symbols):
        raise RuntimeError("Method must be subclassed")


class StaticProcedure(object):
    def __init__(self, name):
        self.name = name


class Arity0Element(DataElement):
    def __init__(self, name):
        super().__init__(name)
        self.elems = {}

    def add(self, elem, value):
        assert len(elem) == 0
        self.elems[elem] = value

    def serialize_data(self, symbols):
        return [serialize_symbol(self.elems[()], symbols)]  # We simply print the only element


class UnaryMap(DataElement):
    ARITY = 1

    def __init__(self, name):
        super().__init__(name)
        self.elems = {}

    def add(self, elem, value):
        self.validate(elem, value)
        self.elems[elem] = value

    def serialize_data(self, symbols):
        return [serialize_tuple(k + (v,), symbols) for k, v in self.elems.items()]

    def validate(self, elem, value):
        if len(elem) != self.ARITY:
            raise RuntimeError("Wrong type or number of arguments for data element {}: {}({}) = {}".format(
                self.name, self.name, elem, value))


class BinaryMap(UnaryMap):
    ARITY = 2


class Arity3Map(BinaryMap):
    ARITY = 3


class Arity4Map(BinaryMap):
    ARITY = 4


class UnarySet(DataElement):
    ARITY = 1

    def __init__(self, name):
        super().__init__(name)
        self.elems = set()

    def add(self, elem, value=None):
        assert value is None
        self.validate(elem)
        self.elems.add(elem)

    def serialize_data(self, symbols):
        return [serialize_tuple(elem, symbols) for elem in self.elems]

    def validate(self, elem):
        if len(elem) != self.ARITY:
            raise RuntimeError("Wrong type or number of arguments for data element {}: {}({})".format(
                self.name, self.name, elem))


class ZeroarySet(UnarySet):
    ARITY = 0

    def __init__(self, name):
        super().__init__(name)
        self.elems = {}

    def add(self, args, value):
        #assert _ is None
        assert args is ()
        self.elems[args] = value

    def serialize_data(self, symbols):
        return [serialize_symbol(self.elems[()], symbols)]  # We simply print the only element


class BinarySet(UnarySet):
    ARITY = 2


class Arity3Set(BinarySet):
    ARITY = 3


class Arity4Set(BinarySet):
    ARITY = 4
