"""
This is a generic PDDL-to-VPM translator.
"""
from compilation.formula_processor import FormulaProcessor
from compilation.schemata import ActionSchemaProcessor

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
        self.init = None
        self.static = None

    def get_instance_name(self):
        return self.task.task_name

    def get_domain_name(self):
        return self.task.domain_name

    def get_types(self):
        elements = []
        for t in self.task.types:
            if t.name != 'object':
                elements.append(base.ObjectType(t.name, t.basetype_name))
        return elements

    def get_symbols(self):
        elements = []
        for s in self.task.predicates:
            argtypes = [t.type for t in s.arguments]
            elements.append(base.Predicate(s.name, argtypes))

        for s in self.task.functions:
            argtypes = [t.type for t in s.arguments]
            elements.append(base.Function(s.name, argtypes, s.type))
        return elements

    def get_action_schemata(self):
        return [ActionSchemaProcessor(self.task, action).process() for action in self.task.actions]

    def get_goal_formula(self):
        return FormulaProcessor(self.task, self.task.goal).process()

    def get_state_constraints(self):
        return FormulaProcessor(self.task, self.task.constraints).process()

    def get_var_from_term(self, term):
        assert all(isinstance(arg, str) and arg in self.task.index.objects.obj_to_idx for arg in term.args)
        return base.Variable(term.symbol, term.args)

    def get_objects(self):
        return [base.ProblemObject(o.name, o.type) for o in self.task.objects]

    def get_initial_state(self):
        return self.init

    def init_data_structures(self):
        init, static = {}, {}
        for f in (f for f in self.task.functions if not is_external(f.name)):
            var = init if f.name in self.task.fluent_symbols else static
            var[f.name] = instantiate_function(f.name, len(f.arguments))
        for p in (p for p in self.task.predicates if not is_external(p.name)):
            var = init if p.name in self.task.fluent_symbols else static
            var[p.name] = instantiate_predicate(p.name, len(p.arguments))
        return init, static

    def process_initial_state(self):
        init, static = self.init_data_structures()

        for atom in self.task.init:
            if isinstance(atom, pddl.Assign):
                name = atom.fluent.symbol
                var = init if name in self.task.fluent_symbols else static
                args = tuple(int(a) if is_int(a) else a for a in atom.fluent.args)
                val = self.parse_value(atom.expression)
                var[name].add(args, val)
            elif isinstance(atom, pddl.Atom):
                name = atom.predicate
                var = init if name in self.task.fluent_symbols else static
                var[name].add(atom.args)
            else:
                raise ValueError("Unexpected atom {}".format(atom))

        self.static = static
        self.init = base.State(init)

    def parse_value(self, expression):
        if isinstance(expression, NumericConstant):
            return expression.value
        else:
            if expression.symbol not in self.task.index.objects.obj_to_idx:
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
