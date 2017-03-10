"""
  A number of classes to deal with FSTRIPS language actions and formulas and compile them into suitable data structures
  that can be afterwards exported to JSON.
"""
import copy

from . import pddl
from .pddl import Effect, Atom, NegatedAtom
from .pddl.effects import AssignmentEffect
from .pddl.f_expression import FunctionalTerm

from . import fstrips as fs
from .parser import Parser


def ensure_conjunction(node):
    # In case we have a single atom, we wrap it on a conjunction
    if isinstance(node, (pddl.conditions.Atom, pddl.conditions.NegatedAtom)):
        node = pddl.conditions.Conjunction([node])
    return node


class Grounding:
    def __init__(self, variable, value):
        self.variable = variable
        self.value = value


def ground_atom(atom, grounding):
    if isinstance(atom, (pddl.Truth, pddl.Falsity)):
        return atom

    if isinstance(atom, str):
        if atom[0] != '?':
            return atom
        if grounding.variable == atom:
            return grounding.value

    if isinstance(atom, AssignmentEffect):
        grounded = copy.deepcopy(atom)
        grounded.lhs = ground_atom(grounded.lhs, grounding)
        grounded.rhs = ground_atom(grounded.rhs, grounding)
        return grounded

    grounded = copy.deepcopy(atom)
    for i, arg in enumerate(atom.args, 0):
        if isinstance(arg, FunctionalTerm):
            grounded_arg = copy.deepcopy(arg)

            # i.e. atom.args[i] = grounding.value
            grounded.args = grounded.args[:i] + (ground_atom(grounded_arg, grounding),) + grounded.args[i+1:]
            continue
        if grounding.variable == arg:
            grounded.args = atom.args[:i] + (grounding.value,) + atom.args[i+1:]  # i.e. atom.args[i] = grounding.value
    return grounded


class BaseComponentProcessor(object):
    def __init__(self, index):
        self.parser = Parser(index)
        self.index = index
        self.data = self.init_data()
        self.binding_unit = fs.BindingUnit()

    def init_data(self):
        raise RuntimeError("Must subclass")

    def process(self):
        raise RuntimeError("Must subclass")

    def process_conditions(self, the_conditions):
        """  Generates the actual conditions from the PDDL parser precondition list"""
        if not the_conditions or isinstance(the_conditions, pddl.conditions.Truth):
            self.data['conditions']['type'] = 'tautology'
        else:
            self.data['conditions'] = self.process_formula(the_conditions)
            self.data['unit'] = self.binding_unit.dump()

    def process_formula(self, node):
        exp = self.parser.process_expression(ensure_conjunction(node), self.binding_unit)
        return exp.dump(self.index.objects, self.binding_unit)


class FormulaProcessor(BaseComponentProcessor):
    def __init__(self, index, formula):
        super().__init__(index)
        self.formula = formula

    def init_data(self):
        return dict(conditions={})

    def process(self):
        self.process_conditions(self.formula)
        return self.data


class ActionSchemaProcessor(BaseComponentProcessor):
    """ A class to dump the information relevant to action schemata """
    def __init__(self, index, action):
        self.action = action
        super().__init__(index)
        self.binding_unit = fs.BindingUnit.from_parameters(action.parameters)

    def init_data(self):
        name = self.action.name
        param_names = [p.name for p in self.action.parameters]
        signature = [self.index.types[p.type] for p in self.action.parameters]
        return dict(name=name, signature=signature, parameters=param_names, conditions={}, effects=[])

    def process(self):
        self.process_conditions(self.action.precondition)
        self.process_effects()
        return self.data

    def ground_possibly_quantified_effect(self, effect):
        assert isinstance(effect, Effect)
        if not effect.parameters:
            return [effect]

        processed = []
        assert len(effect.parameters) == 1, "Only one quantified variable supported ATM"
        parameter = effect.parameters[0]

        for value in self.index.type_map[parameter.type]:
            grounding = Grounding(parameter.name, value)
            processed.append(
                Effect([], ground_atom(effect.condition, grounding), ground_atom(effect.literal, grounding)))

        return processed

    def process_effects(self):
        """  Generates the actual effects from the PDDL parser effect list"""
        for qeffect in self.action.effects:
            for effect in self.ground_possibly_quantified_effect(qeffect):
                effect = self.process_effect(effect.literal, effect.condition)
                self.data['effects'].append(effect)

    def process_effect(self, expression, condition):
        assert isinstance(expression, (AssignmentEffect, Atom, NegatedAtom))

        condition = ensure_conjunction(condition)
        if isinstance(expression, AssignmentEffect):
            lhs, rhs = expression.lhs, expression.rhs
            type_ = 'functional'
        else:
            # The effect has form visited(c), and we want to give it functional form visited(c) := true
            lhs = FunctionalTerm(expression.predicate, expression.args)

            if expression.negated:
                rhs = "0"
                type_ = 'del'
            else:
                rhs = "1"
                type_ = 'add'

        elems = [self.parser.process_expression(elem, self.binding_unit) for elem in (lhs, rhs, condition)]
        return dict(
            lhs=elems[0].dump(self.index.objects, self.binding_unit),
            rhs=elems[1].dump(self.index.objects, self.binding_unit),
            condition=elems[2].dump(self.index.objects, self.binding_unit),
            type=type_
        )
