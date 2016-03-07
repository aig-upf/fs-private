"""
 This module contains a number of basic methods and data structures
 which are necessary to define FSTRIPS domains and problems.
"""
from util import is_int


class Atom(object):
    """ A generic (possibly functional) fact """
    def __init__(self, var, value):
        self.var = var
        self.value = value

    def __hash__(self):
        return hash((self.var, self.value))

    def __eq__(self, other):
        return (self.var, self.value) == (other.var, other.value)

    def __str__(self):
        return '{} = {}'.format(self.var, self.value)
    __repr__ = __str__


class Variable(object):
    """
    A state variable, made up of a symbol and a number of arguments.
    """
    def __init__(self, symbol, args):
        self.symbol = symbol
        self.args = tuple(int(a) if is_int(a) else a for a in args)

    def __hash__(self):
        return hash((self.symbol, self.args))

    def __eq__(self, other):
        return (self.symbol, self.args) == (other.symbol, other.args)

    def __str__(self):
        return '{}{}'.format(self.symbol, "(" + ', '.join(map(str, self.args)) + ")")

    __repr__ = __str__


class Symbol(object):
    def __init__(self, name, arguments):
        self.name = name
        self.arguments = arguments


class Predicate(Symbol):
    def __init__(self, name, domain):
        super().__init__(name, domain)
        self.codomain = 'bool'


class Function(Symbol):
    def __init__(self, name, domain, codomain):
        super().__init__(name, domain)
        self.codomain = codomain


class FunctionInstantiation(object):
    def __init__(self, symbol, mapping=None):
        self.symbol = symbol
        self.mapping = mapping if mapping else {}

    def add(self, point, value):
        assert point not in self.mapping
        self.mapping[point] = value


class PredicateInstantiation(object):
    def __init__(self, symbol, mapping=None):
        self.symbol = symbol
        self.set = mapping if mapping else set()

    def add(self, point):
        assert point not in self.set
        self.set.add(point)


class Parameter(object):
    def __init__(self, name, typename):
        self.name = name
        self.typename = typename

    def __str__(self):
        return '{}: {}'.format(self.name, self.typename)


class Expression(object):
    def __init__(self, symbol, arguments=None):
        self.symbol = symbol
        self.arguments = arguments if arguments else []

    def __str__(self):
        args = "" if self.arguments is None else "({})".format(', '.join(map(str, self.arguments)))
        return "{}{}".format(self.symbol, args)

    def is_fluent(self):
        return not self.is_static()

    def is_static(self):
        return False  # By default, expressions are not static

    def is_subtree_static(self, arguments=None):
        arguments = (self.arguments if self.arguments is not None else []) if arguments is None else arguments
        for arg in arguments:
            if isinstance(arg, FunctionalExpression):
                if not arg.is_static() or not self.is_subtree_static(arg.arguments):
                    return False
        return True

    def is_tree_static(self):
        return self.is_static() and self.is_subtree_static()

    def dump(self, object_index, binding_unit):
        raise RuntimeError("Needs to be subclassed")


class FunctionalExpression(Expression):
    def dump(self, objects, binding_unit):
        subterms = [elem.dump(objects, binding_unit) for elem in self.arguments]
        return dict(type='function', symbol=self.symbol, subterms=subterms)


class StaticFunctionalExpression(FunctionalExpression):
    def is_static(self):
        return True


class PredicativeExpression(Expression):
    def __init__(self, symbol, negated, arguments=None, static=False, type_='atom'):
        super().__init__(symbol, arguments)
        self.negated = negated
        self.static = static
        self.type = type_

    def __str__(self):
        p = Expression.__str__(self)
        return '{}{}'.format("not " if self.negated else "", p)

    def dump_arguments(self, objects, binding_unit):
        elements = [elem.dump(objects, binding_unit) for elem in self.arguments]
        return dict(type=self.type, elements=elements)

    def dump(self, objects, binding_unit):
        d = self.dump_arguments(objects, binding_unit)
        d.update(dict(symbol=self.process_symbol(), negated=self.negated))
        return d

    def process_symbol(self):
        return self.symbol

    def is_static(self):
        return self.static


class ConjunctivePredicate(PredicativeExpression):
    def __init__(self, conjuncts):
        super().__init__(symbol='conjunction', negated=False, arguments=conjuncts, static=True, type_='conjunction')

    def dump(self, objects, binding_unit):
        return self.dump_arguments(objects, binding_unit)


class RelationalExpression(PredicativeExpression):
    def __init__(self, symbol, negated, arguments):
        assert len(arguments) == 2
        super().__init__(symbol, negated, arguments, static=True)

    def process_symbol(self):
        negation_map = {"=": "!=", "<": ">=", "<=": ">", ">": "<=", ">=": "<"}
        return self.symbol if not self.negated else negation_map[self.symbol]


# class ExistentialFormula(PredicativeExpression):
#     # Currently unused
#
#     def __init__(self, parameters, subformula):
#         self.parameters = parameters
#         self.subformula = subformula
#         self.binding_unit = BindingUnit.from_parameters(parameters)
#         super().__init__('exists', False, None, 'existential')
#
#     def dump(self, objects, binding_unit):
#         subformula = self.subformula.dump(objects, binding_unit)
#         return dict(type=self.type, variables=binding_unit.dump_selected(self.parameters), subformula=subformula)


class ArithmeticExpression(StaticFunctionalExpression):
    def __init__(self, symbol, arguments):
        assert len(arguments) == 2
        super().__init__(symbol, arguments)


class ParameterExpression(Expression):
    def __init__(self, name):
        super().__init__(name)

    def dump(self, objects, binding_unit):
        return dict(type='parameter', position=binding_unit.id(self.symbol),
                    typename=binding_unit.typename(self.symbol), name=self.symbol)


class ObjectExpression(Expression):
    def dump(self, objects, binding_unit):
        return dict(type='constant', value=objects.get_index(self.symbol))


class NumericExpression(Expression):
    def dump(self, objects, binding_unit):
        return dict(type='int_constant', value=int(self.symbol))
