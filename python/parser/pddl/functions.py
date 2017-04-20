from . import pddl_types

built_in_functional_symbols = ['+', '-', '*', '/', '^', 'sin', 'cos', 'sqrt', 'tan', 'asin', 'acos', 'atan']

class Function(object):
    def __init__(self, name, arguments):
        self.name = name
        self.arguments = arguments

    @classmethod
    def parse(cls, alist, function_type=None):
        name = alist[0]
        arguments = pddl_types.parse_typed_list(alist[1:],
                                                default_type="number")
        return cls(name, arguments)

    def __str__(self):
        result = "%s(%s)" % (self.name, ", ".join(map(str, self.arguments)))
        return result

    def is_ground(self) :
        for arg in self.args :
            if isinstance( arg, fs_types.TypedObject ) :
                if arg.name[0] == '?' :
                    return False
            elif isinstance(arg, str ) :
                if arg[0] == '?' : return False
            else :
                if not arg.is_ground() : return False
        return True

class TypedFunction(Function):
    """
    A function with arbitrary parameters and return type (aka fluent object)
    """

    def __init__(self, name, arguments, _type):
        super(TypedFunction, self).__init__(name, arguments)
        self.type = _type

    @classmethod
    def parse(cls, alist, function_type=None):
        name = alist[0]
        arguments = pddl_types.parse_typed_list(alist[1:], default_type="object")
        return cls(name, arguments, function_type)

    def __str__(self):
        return "{self.name}({args}):{self.type}".format(self=self, args=", ".join(map(str, self.arguments)))
