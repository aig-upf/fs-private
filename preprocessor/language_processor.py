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
            if isinstance(node, (pddl.conditions.Atom, pddl.conditions.NegatedAtom)):
                # In case we have a single atom, we wrap it on a conjunction
                node = pddl.conditions.Conjunction([node])
            exp = self.parser.process_expression(node)
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

    def process_effects(self):
        """  Generates the actual effects from the PDDL parser effect list"""
        for effect in self.action.effects:
            assert isinstance(effect, Effect) and isinstance(effect.condition, Truth) and not effect.parameters
            effect = self.process_effect(effect.literal)
            self.data['effects'].append(effect)

    def process_effect(self, expression):
        assert isinstance(expression, (AssignmentEffect, Atom, NegatedAtom))

        if isinstance(expression, AssignmentEffect):
            lhs, rhs = expression.lhs, expression.rhs
        else:
            # The effect has form visited(c), and we want to give it functional form visited(c) := true
            lhs = FunctionalTerm(expression.predicate, expression.args)
            rhs = "0" if expression.negated else "1"

        elems = [self.parser.process_expression(elem) for elem in (lhs, rhs)]
        return [elem.dump(self.index.objects, self.binding_unit) for elem in elems]
