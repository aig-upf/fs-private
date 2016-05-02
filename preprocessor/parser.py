"""
    Methods to validate and transform PDDL parser expressions into our convenient data structures.
"""
import exceptions
from base import ParameterExpression, NumericExpression, ObjectExpression, RelationalExpression, \
    ArithmeticExpression, FunctionalExpression, StaticFunctionalExpression, \
    ConjunctivePredicate, PredicativeExpression, Truth
from exceptions import ParseException
from pddl import Atom, NegatedAtom, ExistentialCondition, Conjunction, conditions
from pddl.f_expression import FunctionalTerm
from util import is_int, is_external


def is_relational_operator(symbol):
    return symbol in {"=", "!=", ">", "<", ">=", "<="}


def is_arithmetic_operator(symbol):
    return symbol in {"*", "+", "-", "/"}


def is_builtin_operator(symbol):
    return is_relational_operator(symbol) or is_arithmetic_operator(symbol)


class Parser(object):
    def __init__(self, index):
        self.index = index

    def process_expression(self, exp):
        """  Process an arbitrary expression """
        if isinstance(exp, FunctionalTerm):
            self.check_declared(exp.symbol)
            return self.process_functional_expression(exp)
        elif isinstance(exp, (Atom, NegatedAtom)):
            self.check_declared(exp.predicate)
            return self.process_predicative_expression(exp)
        elif isinstance(exp, ExistentialCondition):
            # return self.process_existential_expression(exp)
            return exp
        elif isinstance(exp, Conjunction):
            return ConjunctivePredicate(self.process_arguments(exp.parts))
        elif isinstance(exp, conditions.Truth):
            return Truth()
        elif isinstance(exp, str):
            if exp[0] == '?':
                return ParameterExpression(exp)
            elif is_int(exp):
                return NumericExpression(exp)
            else:
                return ObjectExpression(exp)
        else:
            raise ParseException("Unknown expression type for expression '{}'".format(exp))

    def is_static(self, symbol):
        return symbol in self.index.static_symbols or is_builtin_operator(symbol) or is_external(symbol)

    # def process_existential_expression(self, exp):
    #     """  Parse an existentially-quantified expression """
    #     assert isinstance(exp, ExistentialCondition)
    #
    #     assert len(exp.parts) == 1, "An existentially quantified formula can have one only subformula"
    #     subformula = exp.parts[0]
    #     return ExistentialFormula(exp.parameters)

    def process_functional_expression(self, exp):
        """  Parse a functional expression """
        assert isinstance(exp, FunctionalTerm)

        if is_arithmetic_operator(exp.symbol):
            return ArithmeticExpression(exp.symbol, self.process_arguments(exp.args))

        c = StaticFunctionalExpression if self.is_static(exp.symbol) else FunctionalExpression
        return c(exp.symbol, self.process_arguments(exp.args))

    def process_predicative_expression(self, exp):
        assert isinstance(exp, (Atom, NegatedAtom))
        name = exp.predicate
        args = self.process_arguments(exp.args)

        if is_external(name) and (exp.negated or not self.is_static(name)):
            raise RuntimeError("External symbols cannot be fluent nor negated")

        if is_relational_operator(name):
            return RelationalExpression(exp.predicate, exp.negated, args)
        else:
            return PredicativeExpression(name, exp.negated, args, static=self.is_static(name))

    def process_arguments(self, args):
        """ Parses a list of predicate / function arguments """
        return [self.process_expression(arg) for arg in args]

    def check_declared(self, symbol):
        if not is_builtin_operator(symbol) and symbol not in self.index.all_symbols and not is_external(symbol):
            raise exceptions.UndeclaredSymbol("Undeclared symbol '{0}'".format(symbol))
