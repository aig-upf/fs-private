"""
    Methods to validate and transform PDDL parser expressions into our convenient data structures.
"""
from copy import deepcopy
import base

from pddl.f_expression import FunctionalTerm
from pddl import Atom, NegatedAtom

from base import ParameterExpression, NumericExpression, ObjectExpression, DefinedExpression, RelationalExpression, \
    ArithmeticExpression, StaticPredicativeExpression, FunctionalExpression, StaticFunctionalExpression, \
    PredicativeExpression, ConstraintExpression
from compilation.exceptions import ParseException
from compilation.helper import is_int


BASE_SYMBOLS = ("=", "!=", "+", "-", ">", "<", ">=", "<=")
CUSTOM_CONSTRAINTS = ("sum_constraint", "alldiff_constraint")


def is_basic_symbol(symbol):
    return symbol in BASE_SYMBOLS or symbol == "@defined"


def is_constraint_expression(symbol):
    return symbol in CUSTOM_CONSTRAINTS


class Parser(object):
    def __init__(self, task):
        self.all = task.all_symbols
        self.static = task.static_symbols
        self.fluent = task.fluent_symbols

    def process(self, exp):
        """
        Processes the whole parse tree represented by the given expression 'exp' and builds a new tree with our custom
        data structures, and where all functional and predicative fluents have been replaced by their corresponding
        state variables.
        """
        return self.process_expression(exp).consolidate_variables()

    def process_expression(self, exp):
        """  Process an arbitrary expression """
        if isinstance(exp, FunctionalTerm):
            self.check_declared(exp.symbol)
            return self.process_functional_expression(exp)
        elif isinstance(exp, (Atom, NegatedAtom)):
            self.check_declared(exp.predicate)
            return self.process_predicative_expression(exp)
        elif isinstance(exp, str):
            if exp[0] == '?':
                return ParameterExpression(exp)
            elif is_int(exp):
                return NumericExpression(exp)
            else:
                return ObjectExpression(exp)
        else:
            raise ParseException("Unknown expression type for expression '{}'".format(exp))

    def process_define_expression(self, exp):
        """ Process the special @define atoms """
        assert len(exp.args) == 1
        return DefinedExpression(exp.negated, self.process_functional_expression(exp.args[0]))

    def process_constraint_expression(self, exp):
        """ Process a constraint expression such as sum(x,y,z). """
        return base.ConstraintExpressionCatalog.instantiate_custom_constraint(exp.predicate, self.process_argument_list(exp.args))

    def process_relational_operator(self, exp):
        """ Process a relational operator such as =, <=, ... """
        return RelationalExpression(exp.predicate, exp.negated, self.process_argument_list(exp.args))

    def process_arithmetic_operator(self, exp):
        """ Process an arithmetic operator such as +, -, ... """
        return ArithmeticExpression(exp.symbol, self.process_argument_list(exp.args))

    def is_static(self, symbol):
        return symbol in self.static or symbol in BASE_SYMBOLS

    def process_functional_expression(self, exp):
        """  Parse a functional expression """
        assert isinstance(exp, FunctionalTerm)

        if is_basic_symbol(exp.symbol):  # A relational operator
            return self.process_arithmetic_operator(exp)

        c = StaticFunctionalExpression if self.is_static(exp.symbol) else FunctionalExpression
        return c(exp.symbol, self.process_argument_list(exp.args))

    def process_predicative_expression(self, exp):
        assert isinstance(exp, (Atom, NegatedAtom))
        if exp.predicate == "@defined":  # An special _is_defined_  predicate
            processed = self.process_define_expression(exp)

        elif is_basic_symbol(exp.predicate):  # A relational operator
            return self.process_relational_operator(exp)

        elif is_constraint_expression(exp.predicate):
            return self.process_constraint_expression(exp)

        else:  # A "standard" predicate
            c = StaticPredicativeExpression if self.is_static(exp.predicate) else PredicativeExpression
            processed = c(exp.predicate, exp.negated, self.process_argument_list(exp.args))

        return processed

    def process_argument_list(self, args):
        """ Parses a list of predicate / function arguments """
        return [self.process_expression(arg) for arg in args]

    def check_declared(self, symbol):
        if not is_basic_symbol(symbol) and not is_constraint_expression(symbol) and symbol not in self.all:
            raise ParseException("Undeclared symbol '{}'".format(symbol))
