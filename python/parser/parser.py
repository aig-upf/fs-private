"""
    Methods to validate and transform PDDL parser expressions into our convenient data structures.
"""
from . import pddl
from .pddl.f_expression import FunctionalTerm

from . import exceptions
from . import fstrips as fs
from .util import is_external


class Parser(object):
    def __init__(self, index):
        self.index = index

    def process_expression(self, exp, binding_unit):
        """
         Parse an arbitrary expression (term or formula), and return a built-in equivalent.
         Might modify the binding unit depending on the type of the expression.
        """
        if isinstance(exp, FunctionalTerm):
            return self.process_functional_expression(exp)
        elif isinstance(exp, (pddl.Atom, pddl.NegatedAtom)):
            self.check_declared(exp.predicate)
            return self.process_atomic_expression(exp)
        elif isinstance(exp, pddl.ExistentialCondition):
            return self.process_quantified_expression(exp, 'exists', binding_unit)
        elif isinstance(exp, pddl.UniversalCondition):
            return self.process_quantified_expression(exp, 'forall', binding_unit)
        elif isinstance(exp, pddl.Conjunction):
            return fs.OpenExpression('conjunction', self.process_children(exp.parts, binding_unit))
        elif isinstance(exp, pddl.Disjunction):
            return fs.OpenExpression('disjunction', self.process_children(exp.parts, binding_unit))
        elif isinstance(exp, pddl.conditions.Truth):
            return fs.Tautology()
        elif isinstance(exp, str):
            if exp[0] == '?':
                return fs.LogicalVariable(exp)
            else:
                return fs.Constant(exp)
        else:
            raise exceptions.ParseException("Unknown expression type for expression '{}'".format(exp))

    def is_static(self, symbol):
        return symbol in self.index.static_symbols or fs.is_builtin_operator(symbol) or is_external(symbol)

    def process_quantified_expression(self, exp, quantifier, binding_unit):
        assert len(exp.parts) == 1, "A quantified expression can have one only subformula"
        binding_unit.merge(fs.BindingUnit.from_parameters(exp.parameters))
        subformula = self.process_expression(exp.parts[0], binding_unit)
        return fs.QuantifiedExpression(quantifier, exp.parameters, subformula)

    def process_functional_expression(self, exp):
        self.check_declared(exp.symbol)
        return fs.FunctionalTerm(exp.symbol, self.process_children(exp.args, None))

    def process_atomic_expression(self, exp):
        name = exp.predicate
        self.check_declared(name)

        if is_external(name) and (exp.negated or not self.is_static(name)):
            raise RuntimeError("External symbols cannot be fluent nor negated")

        return fs.AtomicExpression(name, self.process_children(exp.args, None), exp.negated)

    def process_children(self, children, binding_unit):
        """ Parses a list of predicate / function arguments """
        return [self.process_expression(node, binding_unit) for node in children]

    def check_declared(self, symbol):
        if not fs.is_builtin_operator(symbol) and symbol not in self.index.all_symbols and not is_external(symbol):
            raise exceptions.UndeclaredSymbol("Undeclared symbol '{0}'".format(symbol))
