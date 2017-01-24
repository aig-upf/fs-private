"""
  A number of classes to deal with FSTRIPS language actions and formulas and compile them into suitable data structures
  that can be afterwards exported to JSON.
"""
from collections import namedtuple

import pddl
from pddl import conditions
from pddl import Effect, Truth, Atom, NegatedAtom
from pddl.effects import AssignmentEffect
from pddl.f_expression import FunctionalTerm
from parser import Parser
import copy

TypedVar = namedtuple('TypedVar', ['name', 'type'])


class BindingUnit(object):
    def __init__(self):
        self.typed_vars = []
        self.index = {}  # An index from variable names to (ID, type) tuples

    @staticmethod
    def from_parameters(parameters):
        unit = BindingUnit()
        for param in parameters:
            unit.add(param.name, param.type)
        return unit

    def dump(self):
        return self.dump_selected(self.typed_vars)

    def add(self, name, _type):
        if name in self.index:
            raise RuntimeError("The current binding unit already has a variable with name {}".format(name))

        self.index[name] = (len(self.typed_vars), _type)
        self.typed_vars.append(TypedVar(name, _type))

    def merge(self, binding):
        """ Merge the given binding into the current binding, in-place """
        for var in binding.typed_vars:
            self.add(var.name, var.type)

    def id(self, name):
        """ Return the ID (within the current binding unit) of the parameter with given name """
        return self.index[name][0]

    def typename(self, name):
        """ Return the typename of the parameter with given name """
        return self.index[name][1]

    @staticmethod
    def dump_selected(variables):
        """ Performs a selective dump, dumping only the info about the given variables"""
        return [[i, var.name, var.type] for i, var in enumerate(variables)]


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
    grounded = copy.deepcopy(atom)
    for i, arg in enumerate(atom.args, 0):
        if grounding.variable == arg:
            grounded.args = atom.args[:i] + (grounding.value,) + atom.args[i+1:]  # i.e. atom.args[i] = grounding.value
    return grounded


class BaseComponentProcessor(object):
    def __init__(self, index):
        self.parser = Parser(index)
        self.index = index
        self.data = self.init_data()
        self.binding_unit = BindingUnit()

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

        # ATM we treat existential expressions differently to be able to properly compose the binding unit
        if isinstance(node, pddl.conditions.ExistentialCondition):
            assert len(node.parts) == 1, "An existentially quantified formula can have one only subformula"
            subformula = node.parts[0]
            self.binding_unit.merge(BindingUnit.from_parameters(node.parameters))
            return {'type': 'existential',
                    'variables': self.binding_unit.dump_selected(node.parameters),
                    'subformula': self.process_formula(subformula)}
        else:
            exp = self.parser.process_expression(ensure_conjunction(node))
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
        self.binding_unit = BindingUnit.from_parameters(action.parameters)

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

        elems = [self.parser.process_expression(elem) for elem in (lhs, rhs, condition)]
        return dict(
            lhs=elems[0].dump(self.index.objects, self.binding_unit),
            rhs=elems[1].dump(self.index.objects, self.binding_unit),
            condition=elems[2].dump(self.index.objects, self.binding_unit),
            type=type_
        )
