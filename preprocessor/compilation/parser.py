"""
    Methods to validate and transform PDDL parser expressions into our convenient data structures.
"""
from pddl.f_expression import FunctionalTerm
from pddl import Atom, NegatedAtom, ExistentialCondition

from base import ParameterExpression, NumericExpression, ObjectExpression, RelationalExpression, \
    ArithmeticExpression, StaticPredicativeExpression, FunctionalExpression, StaticFunctionalExpression
from compilation.exceptions import ParseException
from compilation.helper import is_external
from util import is_int


BASE_SYMBOLS = ("=", "!=", "*", "+", "-", ">", "<", ">=", "<=")


def is_basic_symbol(symbol):
    return symbol in BASE_SYMBOLS


class Parser(object):
    def __init__(self, task):
        self.all = task.all_symbols
        self.static = task.static_symbols
        self.fluent = task.fluent_symbols

    def process_expression(self, exp):
        """  Process an arbitrary expression """
        if isinstance(exp, FunctionalTerm):
            self.check_declared(exp.symbol)
            return self.process_functional_expression(exp)
        elif isinstance(exp, (Atom, NegatedAtom)):
            self.check_declared(exp.predicate)
            return self.process_predicative_expression(exp)
        elif isinstance(exp, ExistentialCondition):
            return self.process_existential_expression(exp)
        elif isinstance(exp, str):
            if exp[0] == '?':
                return ParameterExpression(exp)
            elif is_int(exp):
                return NumericExpression(exp)
            else:
                return ObjectExpression(exp)
        else:
            raise ParseException("Unknown expression type for expression '{}'".format(exp))

    def process_relational_operator(self, exp):
        """ Process a relational operator such as =, <=, ... """
        return RelationalExpression(exp.predicate, exp.negated, self.process_argument_list(exp.args))

    def process_arithmetic_operator(self, exp):
        """ Process an arithmetic operator such as +, -, ... """
        return ArithmeticExpression(exp.symbol, self.process_argument_list(exp.args))

    def is_static(self, symbol):
        return symbol in self.static or symbol in BASE_SYMBOLS or is_external(symbol)

    def process_existential_expression(self, exp):
        """  Parse an existentially-quantified expression """
        assert isinstance(exp, ExistentialCondition)
        assert False


    def process_functional_expression(self, exp):
        """  Parse a functional expression """
        assert isinstance(exp, FunctionalTerm)

        if is_basic_symbol(exp.symbol):  # A relational operator
            return self.process_arithmetic_operator(exp)

        c = StaticFunctionalExpression if self.is_static(exp.symbol) else FunctionalExpression
        return c(exp.symbol, self.process_argument_list(exp.args))

    def process_predicative_expression(self, exp):
        assert isinstance(exp, (Atom, NegatedAtom))
        if is_basic_symbol(exp.predicate):  # A relational operator
            result = self.process_relational_operator(exp)

        else:  # A "standard" predicate
            # if self.is_static(exp.predicate):
            #     result = StaticPredicativeExpression(exp.predicate, exp.negated, self.process_argument_list(exp.args))
            # else:  # an atom p is equivalent to a relation p = 1; and similarly for a negated atom, but with p = 0
            if is_external(exp.predicate):
                assert not exp.negated, "Negated external symbols still unimplemented"
                if not self.is_static(exp.predicate):
                    raise RuntimeError("External symbols cannot be fluent")
                result = StaticPredicativeExpression(exp.predicate, exp.negated, self.process_argument_list(exp.args))
            else:
                value = "1" if not exp.negated else "0"
                atom = Atom("=", [FunctionalTerm(exp.predicate, exp.args), value])
                result = self.process_relational_operator(atom)

        return result

    def process_argument_list(self, args):
        """ Parses a list of predicate / function arguments """
        return [self.process_expression(arg) for arg in args]

    def check_declared(self, symbol):
        if not is_basic_symbol(symbol) and \
           symbol not in self.all and \
           not is_external(symbol):
            raise ParseException("Undeclared symbol '{0}'".format(symbol))
