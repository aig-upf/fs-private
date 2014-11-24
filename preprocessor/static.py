"""

"""


def instantiate_function(name, arity):
    classes = {0: Arity0Map, 1: UnaryMap, 2: BinaryMap}
    if arity not in classes:
        raise RuntimeError("Currently only up to arity-2 functions are supported")
    return (classes[arity])(name)


def instantiate_predicate(name, arity):
    classes = {1: UnarySet, 2: BinarySet, 3: Arity3Set}
    if arity not in classes:
        raise RuntimeError("Currently only up to arity-3 predicates are supported")
    return (classes[arity])(name)


class StaticData(object):
    def __init__(self):
        self.declarations = []
        self.accessors = []
        self.initializations = []


# DEPRECATED
# class Tuple(object):
#     def __init__(self, obj1, obj2):
#         self.data = (obj1, obj2)
#
#     def get_init(self, symbols):
#         return '{' + ', '.join(str(symbols[e]) for e in self.data) + '}'


class DataElement:
    def __init__(self, name):
        self.name = name
        self.accessor = 'get_' + name

    def initialization_string(self, t, symbols):
        t = (t,) if not isinstance(t, (list, tuple)) else t
        symbol_id = lambda x: x if isinstance(x, int) else symbols[x]
        inner = ', '.join(str(symbol_id(e)) for e in t)
        return '{' + inner + '}' if len(t) > 1 else inner

    def get_tpl(self, name):
        raise RuntimeError("Method must be subclassed")

    def get_elements(self, name):
        raise RuntimeError("Method must be subclassed")

    def get_accessor(self, symbols):
        return self.get_tpl('accessor').format(**self.__dict__)

    def get_declaration(self, symbols):
        return self.get_tpl('declaration').format(**self.__dict__)

    def get_initialization(self, symbols):
        return self.get_tpl('initialization').format(name=self.name, elems=self.get_elements(symbols))


class StaticProcedure(object):
    def __init__(self, name):
        self.name = name


class Arity0Map(DataElement):
    def get_tpl(self, name):
        return dict(
            declaration='static const unsigned {name};',
            accessor='static unsigned {accessor}() {{ return {name}; }}',
            initialization='const unsigned Ext::{name} = {val};',
        )[name]

    def __init__(self, name):
        super().__init__(name)
        self.elems = {}

    def add(self, elem, value):
        assert len(elem) == 0
        self.elems[elem] = value

    def get_initialization(self, symbols):
        return self.get_tpl('initialization').format(name=self.name, val=self.elems[()])


class UnaryMap(DataElement):
    def get_tpl(self, name):
        return dict(
            declaration='static const std::map<ObjectIdx, unsigned> {name};',
            accessor='static unsigned {accessor}(ObjectIdx x) {{ return {name}.at(x); }}',
            initialization='const std::map<ObjectIdx, unsigned> Ext::{name} = {{\n\t{elems}\n}};',
        )[name]

    def __init__(self, name):
        super().__init__(name)
        self.elems = {}

    def add(self, elem, value):
        self.elems[elem] = value

    def get_elements(self, symbols):
        return ', '.join(self.get_element(k, v, symbols) for k, v in self.elems.items())

    def get_element(self, k, v, symbols):
        return '{' + self.initialization_string(k, symbols) + ', ' + self.initialization_string(v, symbols) + '}'


class BinaryMap(UnaryMap):
    def get_tpl(self, name):
        return dict(
            declaration='static const std::map<std::pair<ObjectIdx, ObjectIdx>, unsigned> {name};',
            accessor='static unsigned {accessor}(ObjectIdx x, ObjectIdx y) {{ return {name}.at({{x,y}}); }}',
            initialization='const std::map<std::pair<ObjectIdx, ObjectIdx>, unsigned> Ext::{name} = {{\n\t{elems}\n}};',
        )[name]


class UnarySet(DataElement):
    def get_tpl(self, name):
        return dict(
            declaration='static const std::set<ObjectIdx> {name};',
            accessor='static bool {accessor}(ObjectIdx x) {{ return {name}.find(x) != {name}.end(); }}',
            initialization='const std::set<ObjectIdx> Ext::{name} = {{\n\t{elems}\n}};',
        )[name]

    def __init__(self, name):
        super().__init__(name)
        self.elems = set()

    def add(self, elem):
        self.elems.add(elem)

    def get_elements(self, symbols):
        return ', '.join(self.get_element(elem, symbols) for elem in self.elems)

    def get_element(self, elem, symbols):
        return self.initialization_string(elem, symbols)


class BinarySet(UnarySet):
    def get_tpl(self, name):
        return dict(
            declaration='static const std::set<std::pair<ObjectIdx, ObjectIdx>> {name};',
            accessor='static bool {accessor}(ObjectIdx x, ObjectIdx y) {{ return {name}.find({{x,y}}) != {name}.end(); }}',
            initialization='const std::set<std::pair<ObjectIdx, ObjectIdx>> Ext::{name} = {{\n\t{elems}\n}};',
        )[name]


class Arity3Set(BinarySet):
    def get_tpl(self, name):
        return dict(
            declaration='static const std::set<std::tuple<ObjectIdx, ObjectIdx, ObjectIdx>> {name};',
            accessor='static bool {accessor}(ObjectIdx x, ObjectIdx y, ObjectIdx z) {{ return {name}.find({{x,y,z}}) != {name}.end(); }}',
            initialization='const std::set<std::tuple<ObjectIdx, ObjectIdx, ObjectIdx>> Ext::{name} = {{\n\t{elems}\n}};',
        )[name]
