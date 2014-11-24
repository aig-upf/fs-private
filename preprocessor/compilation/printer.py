"""
    Some methods to print C++ code.
"""
from compilation.helper import is_external
from base import StaticFunctionalExpression, StaticPredicativeExpression, RelationalExpression, \
    ArithmeticExpression, ParameterExpression, VariableExpression, ObjectExpression, NumericExpression


class CppPrinter(object):
    def __init__(self, index):
        self.index = index

    def print(self, exp):
        """ Print an arbitrary expression """
        # ORDER MATTERS! FROM CONCRETE TO ABSTRACT

        if isinstance(exp, RelationalExpression):
            neg = "!" if exp.negated else ""
            symbol = '==' if exp.symbol == '=' else exp.symbol
            lhs, rhs = self.print_parameters(exp)
            return "{}({} {} {})".format(neg, lhs, symbol, rhs)
        elif isinstance(exp, ArithmeticExpression):
            lhs, rhs = self.print_parameters(exp)
            return "{} {} {}".format(lhs, exp.symbol, rhs)
        elif isinstance(exp, VariableExpression):
            return "relevant[{}]".format(self.index.relevant[exp.variable])
        elif isinstance(exp, ParameterExpression):
            return self.index.parameters[exp.symbol]
        elif isinstance(exp, ObjectExpression):
            return self.index.object_idx.get_index(exp.symbol)
        elif isinstance(exp, NumericExpression):
            return exp.symbol
        elif isinstance(exp, StaticFunctionalExpression):
            arguments = self.print_parameters(exp)
            return self.print_code(exp.symbol, arguments)
        elif isinstance(exp, StaticPredicativeExpression):
            return self.print_code(exp.symbol, self.print_parameters(exp), exp.negated)
        else:
            raise RuntimeError("Unknown expression '{}'".format(exp))

    def print_parameters(self, exp):
        """ Recursively print all the arguments of an expression """
        return [] if exp.arguments is None else [self.print(p) for p in exp.arguments]

    def print_code(self, symbol, params, negated=False):
        """ Helper """
        neg = "! " if negated else ""
        paramstr = ', '.join(params)
        if is_external(symbol):
            name = symbol[1:]
        else:
            name = 'get_' + symbol
        return "{neg}Ext::{name}({params})".format(neg=neg, name=name, params=paramstr)
