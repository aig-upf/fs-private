"""
    A class to dump the information relevant to action schemata
"""
import base
from pddl import Truth, Effect
from pddl.effects import AssignmentEffect
from .helper import get_formula_parts
from compilation.parser import Parser
import util


class ActionSchemaProcessor(object):
    def __init__(self, task, action):
        self.action = action
        self.parameters = {p.name: i for i, p in enumerate(action.parameters)}
        self.parser = Parser(task)
        self.index = task.index
        self.data = dict(name=action.name, classname=util.normalize_action_name(action.name),
                         conditions=[], effects=[])

    def process(self):
        self.process_conditions()
        self.process_effects()
        return self.data

    def process_conditions(self):
        """  Generates the applicability procedures from the PDDL parser precondition list"""
        for part in get_formula_parts(self.action.precondition):
            exp = self.parser.process_expression(part)
            self.data['conditions'].append(exp.dump(self.index.objects, self.parameters))

    def process_effects(self):
        """  Generates the effect procedures from the PDDL parser effect list"""
        for effect in self.action.effects:
            assert isinstance(effect, Effect) and isinstance(effect.condition, Truth) and not effect.parameters
            effect = self.process_effect(effect.literal)
            self.data['effects'].append(effect)

    def process_effect(self, expression):
        if isinstance(expression, AssignmentEffect):  # A functional effect
            elems = [self.parser.process_expression(elem) for elem in (expression.lhs, expression.rhs)]

        else:  # A predicative effect
            lhs = self.parser.process_expression(expression)
            rhs = base.NumericExpression(0 if expression.negated else 1)
            elems = [lhs, rhs]

        return [elem.dump(self.index.objects, self.parameters) for elem in elems]

