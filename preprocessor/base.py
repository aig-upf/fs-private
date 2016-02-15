"""
 This file contains all the necessary entities to define P3R domains and problems.
"""
from collections import OrderedDict

import util
from util import is_action_parameter, is_int


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

    def ground(self, binding):
        args = []
        for a in self.args:
            if is_action_parameter(a):
                args.append(binding[a])
            else:  # Assume it is a constant
                args.append(a)
        return Variable(self.symbol, args)


class ProblemObject(object):
    def __init__(self, name, typename):
        self.name = name
        self.typename = typename


class ObjectType(object):
    def __init__(self, name, parent):
        self.name = name
        self.parent = parent

    def __str__(self):
        return "{name}({parent})".format(**self.__dict__)

    __repr__ = __str__


class Symbol(object):
    def __init__(self, name, arguments):
        self.name = name
        self.arguments = arguments


class Predicate(Symbol):
    def __init__(self, name, domain):
        super().__init__(name, domain)
        self.codomain = '_bool_'


class Function(Symbol):
    def __init__(self, name, domain, codomain):
        super().__init__(name, domain)
        self.codomain = codomain


class FunctionInstantiation(object):
    def __init__(self, symbol, mapping=None):
        self.symbol = symbol
        self.mapping = mapping if mapping else {}

    def add(self, point, value):
        assert not point in self.mapping
        self.mapping[point] = value


class PredicateInstantiation(object):
    def __init__(self, symbol, mapping=None):
        self.symbol = symbol
        self.set = mapping if mapping else set()

    def add(self, point):
        assert not point in self.set
        self.set.add(point)


class Parameter(object):
    def __init__(self, name, typename):
        self.name = name
        self.typename = typename

    def __str__(self):
        return '{}: {}'.format(self.name, self.typename)


class State(object):
    def __init__(self, instantiations):
        self.instantiations = instantiations


class ProblemDomain(object):
    def __init__(self, name, types, supertypes, symbols, symbol_types, schemata):
        self.name = name
        self.types = types
        self.supertypes = supertypes
        self.symbols = self.index_by_name(symbols)
        self.symbol_types = symbol_types
        self.schemata = schemata

    def index_by_name(self, objects):
        """ Index the given objects by their name """
        ordered = OrderedDict()
        for obj in objects:
            ordered[obj.name] = obj
        return ordered

    def get_predicates(self):
        """ Small helper to iterate through the predicates """
        return (s for s in self.symbols.values() if isinstance(s, Predicate))


class ProblemInstance(object):
    def __init__(self, name, domain, objects, init, static_data):
        self.name = name
        self.domain = domain
        self.objects = objects
        self.init = init
        self.static_data = static_data


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
            if isinstance(arg, (FunctionalExpression, VariableExpression)):
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
        return dict(type='nested', symbol=self.symbol, subterms=subterms)


class StaticFunctionalExpression(FunctionalExpression):
    def is_static(self):
        return True


class PredicativeExpression(Expression):
    def __init__(self, symbol, negated, arguments=None, type_='atom'):
        super().__init__(symbol, arguments)
        self.negated = negated
        self.type = type_

    def __str__(self):
        p = Expression.__str__(self)
        return '{}{}'.format("not " if self.negated else "", p)

    def dump(self, objects, binding_unit):
        subterms = [elem.dump(objects, binding_unit) for elem in self.arguments]
        return dict(type=self.type, symbol=self.process_symbol(), subterms=subterms)

    def process_symbol(self):
        return _process_symbol(self.symbol, self.negated)


class StaticPredicativeExpression(PredicativeExpression):
    def is_static(self):
        return True


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


class ConjunctivePredicate(PredicativeExpression):
    def __init__(self, conjuncts):
        super().__init__('conjunction', False, conjuncts, 'conjunction')

    def dump(self, objects, binding_unit):
        conjuncts = [elem.dump(objects, binding_unit) for elem in self.arguments]
        return dict(type=self.type, elements=conjuncts)


class RelationalExpression(StaticPredicativeExpression):
    def __init__(self, symbol, negated, arguments):
        assert len(arguments) == 2
        super().__init__(symbol, negated, arguments)


class ArithmeticExpression(StaticFunctionalExpression):
    def __init__(self, symbol, arguments):
        assert len(arguments) == 2
        super().__init__(symbol, arguments)


class VariableExpression(Expression):
    def __init__(self, variable):
        assert isinstance(variable, Variable)
        super().__init__(variable.symbol, None)
        self.variable = variable

    def __str__(self):
        return str(self.variable)

    def dump(self, objects, binding_unit):
        raise RuntimeError("dump() should never be called on VariableExpressions!")


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


_NEGATED_SYMBOLS = {"=": "!=", "<": ">=", "<=": ">", ">": "<=", ">=": "<"}


def _process_symbol(symbol, negated):
    if not negated:
        return symbol
    return _NEGATED_SYMBOLS[symbol]


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