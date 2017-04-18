"""
 This module contains a number of basic methods and data structures
 which are necessary to define FSTRIPS domains and problems.
"""
from collections import namedtuple

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
    def __init__(self, symbol, children):
        super().__init__(symbol)
        self.children = children

    def dump(self, index, binding_unit):
        children = [elem.dump(index, binding_unit) for elem in self.children]
        return dict(type='functional', symbol=self.symbol, children=children)

    def __str__(self):
        return "{} ({})".format(self.symbol, ','.join(str(child) for child in self.children))


class LogicalVariable(Term):
    def __init__(self, name):
        super().__init__(name)

    def dump(self, index, binding_unit):
        return dict(type='variable', position=binding_unit.id(self.symbol),
                    typename=binding_unit.typename(self.symbol), name=self.symbol)

    def __str__(self):
        return "{}".format(self.symbol)


class Constant(Term):
    def dump(self, index, binding_unit):
        if is_int(self.symbol):  # We have a numeric constant
            return dict(type='int_constant', value=int(self.symbol), typename="int")

        else:  # We have a logical constant
            return dict(type='constant', value=index.objects.get_index(self.symbol),
                        typename=index.object_types[self.symbol])

    def __str__(self):
        return "{}".format(self.symbol)


class Relation(object):
    def __init__(self, head):
        self.head = head


class Tautology(object):
    def dump(self, *args):
        return dict(type='tautology')

    def __str__(self):
        return "True"


class AtomicExpression(Relation):
    def __init__(self, symbol, children, negated):  # TODO - Deprecate negated and use negation instead
        super().__init__(symbol)
        self.children = children
        self.negated = negated

    def dump(self, index, binding_unit):
        sym, neg = negate_if_possible(self.head, self.negated)
        children = [elem.dump(index, binding_unit) for elem in self.children]
        return dict(type='atom', symbol=sym, children=children, negated=neg)

    def __str__(self):
        neg = "not " if self.negated else ""
        return "{}{}({})".format(neg, self.head, ','.join(str(child) for child in self.children))


class OpenExpression(Relation):
    CONNECTIVES = {'and', 'or', 'not'}

    def __init__(self, connective, children):
        assert connective in self.CONNECTIVES
        super().__init__(connective)
        self.children = children

    def dump(self, index, binding_unit):
        children = [elem.dump(index, binding_unit) for elem in self.children]
        return dict(type=self.head, symbol=self.head, children=children, negated=False)

    def __str__(self):
        return "{}({})".format(self.head, ', '.join(str(child) for child in self.children))


class QuantifiedExpression(Relation):
    QUANTIFIERS = {'exists', 'forall'}

    def __init__(self, quantifier, variables, subformula):
        assert quantifier in self.QUANTIFIERS
        super().__init__(quantifier)
        self.variables = variables
        self.subformula = subformula

    def dump(self, index, binding_unit):
        subformula = self.subformula.dump(index, binding_unit)
        return dict(type=self.head, variables=binding_unit.dump_selected(self.variables), subformula=subformula)

    def __str__(self):
        return "{}_({}) ({})".format(self.head, ','.join(str(child) for child in self.variables), self.subformula)


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

    def clone(self):
        unit = BindingUnit()
        unit.typed_vars = self.typed_vars[:]
        unit.index = self.index.copy()
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

    def dump_selected(self, variables):
        """ Performs a selective dump, dumping only the info about the given variables"""
        return [[self.index[var.name][0], var.name, var.type] for var in variables]


def to_prenex_normal_form(exp):
    """
      Parse an arbitrary expression (term or formula), and return a built-in equivalent.
      Might modify the binding unit depending on the type of the expression.
     """
    if isinstance(exp, Term):
        return exp

    elif isinstance(exp, (AtomicExpression, Tautology)):
        return exp

    if isinstance(exp, OpenExpression) and exp.head == 'not':
        raise RuntimeError("UNIMPLEMENTED")

    elif isinstance(exp, OpenExpression):
        assert exp.head in ('and', 'or')
        pnf_expression = exp

        # First put all children of the open formula in PNF
        pnf_expression.children = [to_prenex_normal_form(child) for child in pnf_expression.children]

        # The recursively ensure that all quantifiers are pushed up
        for i, quantified in enumerate(pnf_expression.children):
            if isinstance(quantified, QuantifiedExpression):
                pnf_expression.children[i] = quantified.subformula
                head = QuantifiedExpression(quantified.head, quantified.variables, pnf_expression)
                return to_prenex_normal_form(head)

        # If we reach this point, there are no quantified expressions in the subtree rooted at this open formula
        return pnf_expression

    elif isinstance(exp, QuantifiedExpression):
        exp.subformula = to_prenex_normal_form(exp.subformula)
        return exp

    else:
        raise RuntimeError("Unknown expression type for expression '{}'".format(exp))
