"""
    This module contains a number of classes and routines to handle the static (external) data, including its
    declaration and serialization.
"""


def instantiate_function(name, arity):
    classes = {0: Arity0Element, 1: UnaryMap, 2: BinaryMap, 3: Arity3Map, 4: Arity4Map}
    if arity not in classes:
        raise RuntimeError("Currently only up to arity-2 functions are supported")
    return (classes[arity])(name)


def instantiate_predicate(name, arity):
    classes = {1: UnarySet, 2: BinarySet, 3: Arity3Set, 4: Arity4Set}
    if arity not in classes:
        raise RuntimeError("Currently only up to arity-3 predicates are supported")
    return (classes[arity])(name)


class StaticData(object):
    def __init__(self):
        self.declarations = []
        self.accessors = []
        self.initializations = []


def serialize_tuple(t, symbols):
    """  A small helper to serialize a whole tuple with the help of a symbol table """
    t = (t,) if not isinstance(t, (list, tuple)) else t
    symbol_id = lambda x: x if isinstance(x, int) else symbols[x]
    return ','.join(str(symbol_id(e)) for e in t)


class DataElement:
    DESERIALIZER = None

    def __init__(self, name):
        self.name = name
        self.accessor = 'get_' + name

    def get_tpl(self, name):
        raise RuntimeError("Method must be subclassed")

    def get_accessor(self, symbols):
        return self.get_tpl('accessor').format(**self.__dict__)

    def get_declaration(self, symbols):
        return self.get_tpl('declaration').format(name=self.name)

    def serialize_data(self, symbols):
        raise RuntimeError("Method must be subclassed")

    def initializer_list(self):
        assert self.DESERIALIZER is not None
        tpl = '{name}(Serializer::{deserializer}(data_dir + "/{name}.data"))'
        return tpl.format(name=self.name, deserializer=self.DESERIALIZER)


class StaticProcedure(object):
    def __init__(self, name):
        self.name = name


class Arity0Element(DataElement):
    DESERIALIZER = 'deserialize0AryElement'

    def get_tpl(self, name):
        return dict(
            declaration='const ObjectIdx {name};',
            accessor='ObjectIdx {accessor}() {{ return {name}; }}',
        )[name]

    def __init__(self, name):
        super().__init__(name)
        self.elems = {}

    def add(self, elem, value):
        assert len(elem) == 0
        self.elems[elem] = value

    def get_declaration(self, symbols):
        return self.get_tpl('declaration').format(name=self.name, val=self.elems[()])

    def serialize_data(self, symbols):
        return [str(self.elems[()])]  # We simply print the only element


class UnaryMap(DataElement):
    DESERIALIZER = 'deserializeUnaryMap'

    def get_tpl(self, name):
        return dict(
            declaration='const std::map<ObjectIdx, ObjectIdx> {name};',
            accessor='ObjectIdx {accessor}(ObjectIdx x) {{ return {name}.at(x); }}',
        )[name]

    def __init__(self, name):
        super().__init__(name)
        self.elems = {}

    def add(self, elem, value):
        self.elems[elem] = value

    def serialize_data(self, symbols):
        return [serialize_tuple(k + (v,), symbols) for k, v in self.elems.items()]


class BinaryMap(UnaryMap):
    DESERIALIZER = 'deserializeBinaryMap'

    def get_tpl(self, name):
        return dict(
            declaration='const std::map<std::pair<ObjectIdx, ObjectIdx>, ObjectIdx> {name};',
            accessor='ObjectIdx {accessor}(ObjectIdx x, ObjectIdx y) {{ return {name}.at({{x,y}}); }}',
        )[name]


class Arity3Map(BinaryMap):
    DESERIALIZER = 'deserializeArity3Map'

    def get_tpl(self, name):
        return dict(
            declaration='const std::map<std::tuple<ObjectIdx, ObjectIdx, ObjectIdx>, ObjectIdx> {name};',
            accessor='ObjectIdx {accessor}(ObjectIdx x, ObjectIdx y, ObjectIdx z) {{ return {name}.at(std::make_tuple(x,y,z)); }}',
        )[name]


class Arity4Map(BinaryMap):
    DESERIALIZER = 'deserializeArity4Map'

    def get_tpl(self, name):
        return dict(
            declaration='const std::set<std::tuple<ObjectIdx, ObjectIdx, ObjectIdx, ObjectIdx>, ObjectIdx> {name};',
            accessor='ObjectIdx {accessor}(ObjectIdx o1, ObjectIdx o2, ObjectIdx o3, ObjectIdx o4) {{ return {name}.at(std::make_tuple(o1,o2,o3,o4)); }}',
        )[name]


class UnarySet(DataElement):
    DESERIALIZER = 'deserializeUnarySet'

    def get_tpl(self, name):
        return dict(
            declaration='const std::set<ObjectIdx> {name};',
            accessor='bool {accessor}(ObjectIdx x) {{ return {name}.find(x) != {name}.end(); }}',
        )[name]

    def __init__(self, name):
        super().__init__(name)
        self.elems = set()

    def add(self, elem):
        self.elems.add(elem)

    def serialize_data(self, symbols):
        return [serialize_tuple(elem, symbols) for elem in self.elems]


class BinarySet(UnarySet):
    DESERIALIZER = 'deserializeBinarySet'

    def get_tpl(self, name):
        return dict(
            declaration='const std::set<std::pair<ObjectIdx, ObjectIdx>> {name};',
            accessor='bool {accessor}(ObjectIdx x, ObjectIdx y) {{ return {name}.find({{x,y}}) != {name}.end(); }}',
        )[name]


class Arity3Set(BinarySet):
    DESERIALIZER = 'deserializeArity3Set'

    def get_tpl(self, name):
        return dict(
            declaration='const std::set<std::tuple<ObjectIdx, ObjectIdx, ObjectIdx>> {name};',
            accessor='bool {accessor}(ObjectIdx x, ObjectIdx y, ObjectIdx z) {{ return {name}.find(std::make_tuple(x,y,z)) != {name}.end(); }}',
        )[name]


class Arity4Set(BinarySet):
    DESERIALIZER = 'deserializeArity4Set'

    def get_tpl(self, name):
        return dict(
            declaration='const std::set<std::tuple<ObjectIdx, ObjectIdx, ObjectIdx, ObjectIdx>> {name};',
            accessor='bool {accessor}(ObjectIdx o1, ObjectIdx o2, ObjectIdx o3, ObjectIdx o4) {{ return {name}.find(std::make_tuple(o1,o2,o3,o4)) != {name}.end(); }}',
        )[name]

