"""
"""
from collections import namedtuple
import pddl
from compilation.parser import Parser


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
    def __init__(self, task):
        self.parser = Parser(task)
        self.index = task.index
        self.data = self.init_data()
        self.binding_unit = BindingUnit()

    def init_data(self):
        raise RuntimeError("Must subclass")

    def process(self):
        raise RuntimeError("Must subclass")

    def process_conditions(self, conditions):
        """  Generates the actual conditions from the PDDL parser precondition list"""
        if not conditions or isinstance(conditions, pddl.conditions.Truth):
            self.data['conditions']['type'] = 'tautology'
        else:
            self.data['conditions'] = self.process_formula(conditions)
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
            exp = self.parser.process_expression(node)
            return exp.dump(self.index.objects, self.binding_unit)
