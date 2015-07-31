"""
"""
import base
from pddl import Truth, Effect
from pddl.effects import AssignmentEffect
from .helper import get_formula_parts
from compilation.parser import Parser
import util


class BaseComponentProcessor(object):
    def __init__(self, task):
        self.parser = Parser(task)
        self.index = task.index
        self.data = self.init_data()

    def init_data(self):
        raise RuntimeError("Must subclass")

    def process(self):
        raise RuntimeError("Must subclass")

    def get_parameters(self):
        raise RuntimeError("Must subclass")

    def process_conditions(self, conditions):
        """  Generates the applicability procedures from the PDDL parser precondition list"""
        if conditions:
            for part in get_formula_parts(conditions):
                exp = self.parser.process_expression(part)
                self.data['conditions'].append(exp.dump(self.index.objects, self.get_parameters()))

