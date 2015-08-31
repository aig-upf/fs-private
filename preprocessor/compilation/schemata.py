"""
    A class to dump the information relevant to action schemata
"""
from compilation.component_processor import BaseComponentProcessor
from pddl import Truth, Effect, Atom, NegatedAtom
from pddl.f_expression import FunctionalTerm
from pddl.effects import AssignmentEffect


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
        return dict(name=name, signature=signature, parameters=params,
                    conditions=[], effects=[])

    def process(self):
        self.process_conditions(self.action.precondition)
        self.process_effects()
        return self.data

    def get_parameters(self):
        return self.parameters

    def process_effects(self):
        """  Generates the actual effects from the PDDL parser effect list"""
        for effect in self.action.effects:
            assert isinstance(effect, Effect) and isinstance(effect.condition, Truth) and not effect.parameters
            effect = self.process_effect(effect.literal)
            self.data['effects'].append(effect)

    def process_effect(self, expression):
        assert isinstance(expression, (AssignmentEffect, Atom, NegatedAtom))

        if isinstance(expression, (Atom, NegatedAtom)):
            # The effect has form visited(c), and we want to give it functional form visited(c) := true
            lhs = FunctionalTerm(expression.predicate, expression.args)
            rhs = "0" if expression.negated else "1"
            expression = AssignmentEffect(lhs, rhs)

        elems = [self.parser.process_expression(elem) for elem in (expression.lhs, expression.rhs)]
        return [elem.dump(self.index.objects, self.parameters) for elem in elems]

