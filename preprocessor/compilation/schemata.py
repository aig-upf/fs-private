"""
    A class to dump the information relevant to action schemata
"""
import base
from compilation.component_processor import BaseComponentProcessor
from pddl import Truth, Effect
from pddl.effects import AssignmentEffect
import util


class ActionSchemaProcessor(BaseComponentProcessor):
    def __init__(self, task, action):
        self.index = task.index
        self.action = action
        self.parameters = {p.name: i for i, p in enumerate(action.parameters)}
        super().__init__(task)

    def init_data(self):
        name = self.action.name
        params = list(self.parameters.keys())
        signature = [self.index.types[p.type] for p in self.action.parameters]
        return dict(name=name, classname=util.normalize_action_name(name), signature=signature, parameters=params,
                    conditions=[], effects=[])

    def process(self):
        self.process_conditions(self.action.precondition)
        self.process_effects()
        return self.data

    def get_parameters(self):
        return self.parameters

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

