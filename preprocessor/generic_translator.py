"""
This is a generic PDDL-to-VPM translator.
"""
from compilation.formula_processor import FormulaProcessor

import pddl  # This should be imported from a custom-set PYTHONPATH containing the path to Fast Downward's PDDL parser
from pddl.f_expression import NumericConstant

import base
from compilation.exceptions import ParseException
from compilation.helper import is_external
from static import StaticProcedure, instantiate_function, instantiate_predicate
from util import is_int


class Translator(object):
    def __init__(self, task):
        assert(isinstance(task, pddl.tasks.Task))
        self.task = task
        self.static = None

    def get_instance_name(self):
        return self.task.task_name

    def get_domain_name(self):
        return self.task.domain_name

    def get_goal_formula(self):
        return FormulaProcessor(self.task, self.task.goal).process()

    def get_state_constraints(self):
        return FormulaProcessor(self.task, self.task.constraints).process()

    def get_var_from_term(self, term):
        assert all(isinstance(arg, str) and arg in self.task.index.objects for arg in term.args)
        return base.Variable(term.symbol, term.args)

    def get_objects(self):
        return [base.ProblemObject(o.name, o.type) for o in self.task.objects]

    def init_data_structures(self, symbols):
        init, static = {}, {}
        for s in symbols.values():
            if is_external(s.name):
                continue

            var = init if s.name in self.task.fluent_symbols else static
            if isinstance(s, base.Predicate):
                var[s.name] = instantiate_predicate(s.name, len(s.arguments))
            else:  # We have a function
                var[s.name] = instantiate_function(s.name, len(s.arguments))
        return init, static

    def process_initial_state(self, symbols):
        init, static = self.init_data_structures(symbols)

        def check_symbol(s):  # A small helper
            if s == 'total-cost':
                return False
            if s not in symbols:
                raise ParseException("Unknown symbol: '{}'".format(s))
            return True

        for atom in self.task.init:
            if isinstance(atom, pddl.Assign):
                name = atom.fluent.symbol
                if check_symbol(name):
                    var = init if name in self.task.fluent_symbols else static
                    args = tuple(int(a) if is_int(a) else a for a in atom.fluent.args)
                    val = self.parse_value(atom.expression)
                    var[name].add(args, val)
            elif isinstance(atom, pddl.Atom):
                assert not atom.negated, "No negations allowed in the initialization of atoms"
                name = atom.predicate
                if check_symbol(name):
                    var = init if name in self.task.fluent_symbols else static
                    var[name].add(atom.args)
            else:
                raise ValueError("Unexpected atom {}".format(atom))

        self.static = static
        return base.State(init)

    def parse_value(self, expression):
        if isinstance(expression, NumericConstant):
            return expression.value
        else:
            if expression.symbol not in self.task.index.objects:
                raise ParseException("Functions need to be instantiated to plain objects")
            return expression.symbol

    def get_static_data(self):
        data = self.static
        processed = {}

        for k, v in data.items():
            if is_external(k):
                processed[k] = StaticProcedure(k)
            else:
                processed[k] = v
        return processed
