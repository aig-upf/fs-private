"""
 This module contains a number of basic methods and data structures
 which are necessary to define FSTRIPS domains and problems.
"""
from collections import namedtuple
from enum import Enum

from python.utils import is_int


def is_relational_operator(symbol):
    return symbol in {"=", "!=", ">", "<", ">=", "<="}


def is_arithmetic_function(symbol):
    return symbol in {"*", "+", "-", "/"}


def is_builtin_operator(symbol):
    return is_relational_operator(symbol) or is_arithmetic_function(symbol)


def negate_predicate(symbol):
    """
    Return the inverse (i.e. the equivalent to its negation) of the given relation symbol, if any, otherwise None.
    """
    negation_map = {"=": "!=", "<": ">=", "<=": ">", ">": "<=", ">=": "<"}
    return negation_map.get(symbol, None)


def negate_if_possible(symbol, negated):
    """ Return a tuple indicating whether the actual symbol and whether the resulting formula is negated or not. """
    negated_symbol = negate_predicate(symbol)

    if negated and negated_symbol:
        return negated_symbol, True

    return symbol, negated


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


class Term(object):
    def __init__(self, symbol):
        self.symbol = symbol


class FunctionalTerm(Term):
    def __init__(self, symbol, arguments):
        super().__init__(symbol)
        self.arguments = arguments

    def dump(self, objects, binding_unit):
        children = [elem.dump(objects, binding_unit) for elem in self.arguments]
        return dict(type='function', symbol=self.symbol, children=children)


class LogicalVariable(Term):
    def __init__(self, name):
        super().__init__(name)

    def dump(self, objects, binding_unit):
        return dict(type='parameter', position=binding_unit.id(self.symbol),
                    typename=binding_unit.typename(self.symbol), name=self.symbol)


class Constant(Term):
    def dump(self, objects, binding_unit):
        if is_int(self.symbol):  # We have a numeric constant
            return dict(type='int_constant', value=int(self.symbol))

        else:  # We have a logical constant
            return dict(type='constant', value=objects.get_index(self.symbol))


class Tautology(object):
    def dump(self, objects, binding_unit):
        return dict(type='tautology')


class Relation(object):
    def __init__(self, head):
        self.head = head


class AtomicExpression(Relation):
    def __init__(self, symbol, children, negated):  # TODO - Deprecate negated and use negation instead
        super().__init__(symbol)
        self.children = children
        self.negated = negated

    def dump(self, objects, binding_unit):
        sym, neg = negate_if_possible(self.head, self.negated)
        children = [elem.dump(objects, binding_unit) for elem in self.children]
        return dict(type='atom', symbol=sym, children=children, negated=neg)


class OpenExpression(Relation):
    CONNECTIVE = Enum('CONNECTIVE', 'AND OR NOT')
    KEY = {CONNECTIVE.AND: 'conjunction', CONNECTIVE.OR: 'disjunction', CONNECTIVE.NOT: 'negation'}

    def __init__(self, connective, children):
        super().__init__(connective)
        self.children = children

    def dump(self, objects, binding_unit):
        name = self.KEY[self.head]
        children = [elem.dump(objects, binding_unit) for elem in self.children]
        return dict(type=name, symbol=name, children=children, negated=False)


class QuantifiedExpression(Relation):
    QUANTIFIER = Enum('QUANTIFIER', 'EXISTS FORALL')

    def __init__(self, quantifier, variables, subformula):
        super().__init__(quantifier)
        self.variables = variables
        self.subformula = subformula
        self.binding_unit = BindingUnit.from_parameters(variables)

    def dump(self, objects, binding_unit):
        subformula = self.subformula.dump(objects, binding_unit)
        return dict(type=self.head.name, variables=binding_unit.dump_selected(self.variables), subformula=subformula)


TypedVar = namedtuple('TypedVar', ['name', 'type'])


class BindingUnit(object):
    def __init__(self):
        self.typed_vars = []
        self.index = {}  # An index from variable names to (ID, type) tuples

    @staticmethod
    def from_parameters(parameters):
        unit = BindingUnit()
        for param in parameters:
            unit.add(param.name, param.type)
        return unit

    def dump(self):
        return self.dump_selected(self.typed_vars)

    def add(self, name, _type):
        if name in self.index:
            raise RuntimeError("The current binding unit already has a variable with name {}".format(name))

        self.index[name] = (len(self.typed_vars), _type)
        self.typed_vars.append(TypedVar(name, _type))

    def merge(self, binding):
        """ Merge the given binding into the current binding, in-place """
        for var in binding.typed_vars:
            self.add(var.name, var.type)

    def id(self, name):
        """ Return the ID (within the current binding unit) of the parameter with given name """
        return self.index[name][0]

    def typename(self, name):
        """ Return the typename of the parameter with given name """
        return self.index[name][1]

    @staticmethod
    def dump_selected(variables):
        """ Performs a selective dump, dumping only the info about the given variables"""
        return [[i, var.name, var.type] for i, var in enumerate(variables)]
