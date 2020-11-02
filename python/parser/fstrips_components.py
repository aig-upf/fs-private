"""
  A number of classes to deal with FSTRIPS language actions and formulas and compile them into suitable data structures
  that can be afterwards exported to JSON.
"""
import copy

from . import pddl
from .pddl.f_expression import FunctionalTerm
from .pddl.conditions import Conjunction
from . import fstrips as fs
from .parser import Parser, exceptions


def ensure_conjunction(node):
    # In case we have a single atom, we wrap it on a conjunction
    if isinstance(node, (pddl.Atom, pddl.NegatedAtom)):
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

    grounded = copy.deepcopy(atom)
    if isinstance(atom, pddl.effects.AssignmentEffect):
        grounded.lhs = ground_atom(grounded.lhs, grounding)
        grounded.rhs = ground_atom(grounded.rhs, grounding)
        return grounded

    if isinstance(atom, Conjunction):
        grounded_parts = []
        for p in atom.parts:
            grounded_parts.append(ground_atom(p, grounding))
        return pddl.conditions.Conjunction(grounded_parts)

    for i, arg in enumerate(atom.args, 0):
        if isinstance(arg, FunctionalTerm):
            grounded_arg = copy.deepcopy(arg)

            # i.e. atom.args[i] = grounding.value
            grounded.args = grounded.args[:i] + (ground_atom(grounded_arg, grounding),) + grounded.args[i+1:]
            continue
        if grounding.variable == arg:
            grounded.args = atom.args[:i] + (grounding.value,) + atom.args[i+1:]  # i.e. atom.args[i] = grounding.value
    return grounded


def ground_possibly_quantified_effect(effect, type_map):
    """
       Ground a "universally-quantified" effect into its logical expansion.
       Note that this type of universally-quantified effects are syntactic sugar to encode a large
       set of effects in a compact manner, but are different to having universal quantification within a language
       formula.
    """
    assert isinstance(effect, pddl.Effect)
    if not effect.parameters:
        return [effect]

    processed = []
    parameter = effect.parameters[0]
    remaining = effect.parameters[1:]

    # We ground effects with multiple variables recursively.
    for value in type_map[parameter.type]:
        grounding = Grounding(parameter.name, value)
        grounded_effect = pddl.Effect(remaining, ground_atom(effect.condition, grounding), ground_atom(effect.literal, grounding))
        processed += ground_possibly_quantified_effect(grounded_effect, type_map)

    return processed


class FSBaseComponent(object):
    """ A useful base class for both FSTRIPS formulas and actions """
    def __init__(self, index):
        self.parser = Parser(index)
        self.index = index
        self.binding_unit = fs.BindingUnit()

    def process_conditions(self, conditions):
        """  Generates the actual conditions from the PDDL parser precondition list"""
        if not conditions or isinstance(conditions, pddl.conditions.Truth):
            return fs.Tautology
        else:
            return self.parser.process_expression(ensure_conjunction(conditions), self.binding_unit)


class FSFormula(FSBaseComponent):
    """ A FSTRIPS formula (goal, state constraint, action precondition, etc) """
    def __init__(self, index, formula):
        super().__init__(index)
        self.formula = formula
        self.processed = self.process_conditions(self.formula)

    def dump(self):
        return dict(conditions=self.processed.dump(self.index, self.binding_unit),
                    unit=self.binding_unit.dump())

class FSMetric(FSBaseComponent) :
    """ A state--dependant metric (i.e. a expression to optimise defined over state variables)"""
    def __init__(self, index, opt_mode, expr ) :
        super().__init__(index)
        if opt_mode is None or expr == (None, None):
            self.opt_mode = self.expr = None
            return
        self.expr = expr

        if self.expr[0] is not None :
            self.terminal_cost = self.parser.process_expression(expr[0], self.binding_unit)
        if self.expr[1] is not None :
            self.stage_cost = self.parser.process_expression(expr[1], self.binding_unit)
        self.opt_mode = opt_mode

    def dump(self) :
        if self.opt_mode is None :
            return dict()
        if self.expr[0] is not None and self.expr[1] is not None :
            return dict(optimization = self.opt_mode,
                        terminal_cost=self.terminal_cost.dump(self.index,self.binding_unit),
                        stage_cost=self.stage_cost.dump(self.index,self.binding_unit))
        elif self.expr[0] is None and self.expr[1] is not None :
            return dict(optimization = self.opt_mode,
                        stage_cost=self.stage_cost.dump(self.index,self.binding_unit))
        elif self.expr[0] is not None and self.expr[1] is None :
            return dict(optimization = self.opt_mode,
                        terminal_cost=self.terminal_cost.dump(self.index,self.binding_unit))
        elif self.expr[0] is  None and self.expr[1] is None :
            return dict()
        assert False

class FSNamedFormula(FSBaseComponent):
    """ A FSTRIPS axiom, which is a formula with a name and possibly some lifted parameters """
    def __init__(self, index, name, parameters, formula):
        super().__init__(index)
        self.name = name
        self.parameters = parameters

        # Order matters: the binding unit needs to be created when the effects are processed
        self.binding_unit = fs.BindingUnit.from_parameters(self.parameters)
        self.formula = self.process_conditions(formula)

    def dump(self):
        return dict(name=self.name,
            signature=[self.index.types[p.type] for p in self.parameters],
            parameters=[p.name for p in self.parameters],
            conditions=self.formula.dump(self.index, self.binding_unit),
            unit=self.binding_unit.dump())

    def __str__(self):
        if self.parameters:
            params = "({})".format(', '.join("{}: {}".format(p.name, p.type) for p in self.parameters))
        else:
            params = ""
        return "{}{}\n\t{}".format(self.name, params, self.formula)


class FSActionSchema(FSBaseComponent):
    """ A FSTRIPS action schema """
    def __init__(self, index, action, type_ = "control"):
        super().__init__(index)
        self.action = action
        self.type = type_

        # Order matters: the binding unit needs to be created when the effects are processed
        self.binding_unit = fs.BindingUnit.from_parameters(action.parameters)
        try :
            self.precondition = self.process_conditions(self.action.precondition)
        except exceptions.UndeclaredSymbol as e:
            raise SystemExit('Undeclared symbol in precondition of schema "{}", exception message:\n{}'.format(action.name,e,))

        try :
            self.effects = self.process_effects()
        except exceptions.UndeclaredSymbol as e :
            raise SystemExit('Undeclared symbol in effects of schema "{}", exception message: \n {}'.format(action.name,e))


    def dump(self):
        return dict(name=self.action.name,
                    signature=[self.index.types[p.type] for p in self.action.parameters],
                    type =self.type,
                    parameters=[p.name for p in self.action.parameters],
                    conditions=self.precondition.dump(self.index, self.binding_unit),
                    effects=[eff.dump() for eff in self.effects],
                    unit=self.binding_unit.dump())

    def process_effects(self):
        """  Generates the actual effects from the PDDL parser effect list"""
        processed = []
        for qeffect in self.action.effects:
            for effect in ground_possibly_quantified_effect(qeffect, self.index.type_map):
                processed.append(self.process_effect(effect.literal, effect.condition))
        return processed

    def process_effect(self, expression, condition):
        assert isinstance(expression, (pddl.effects.AssignmentEffect, pddl.Atom, pddl.NegatedAtom))

        # Each effect needs a distinct binding unit that contains all the variables from the action global
        # binding unit plus, eventually, new variables that appear in the effect's conditions, etc.
        binding_unit = self.binding_unit.clone()

        condition = ensure_conjunction(condition)
        if isinstance(expression, pddl.effects.AssignmentEffect):
            lhs, rhs = expression.lhs, expression.rhs
            type_ = 'functional'
        else:
            # The effect has form visited(c), and we want to give it functional form visited(c) := true
            # TODO - THIS MUST GO AWAY - and will cause problems soon :-)
            lhs = FunctionalTerm(expression.predicate, expression.args)

            if expression.negated:
                rhs = "false"
                type_ = 'del'
            else:
                rhs = "true"
                type_ = 'add'

        lhs, rhs, condition = (self.parser.process_expression(elem, binding_unit) for elem in (lhs, rhs, condition))

        return FSActionEffect(lhs, rhs, condition, type_, index=self.index, binding_unit=binding_unit)


class FSActionEffect(object):
    """ The effect of a FS action """
    def __init__(self, lhs, rhs, condition, type_, index, binding_unit):
        self.lhs = lhs
        self.rhs = rhs
        self.condition = condition
        self.type_ = type_
        self.index = index
        self.binding_unit = binding_unit

    def dump(self):
        # print("{} --> {} := {}".format(self.condition, self.lhs, self.rhs))
        return dict(
            lhs=self.lhs.dump(self.index, self.binding_unit),
            rhs=self.rhs.dump(self.index, self.binding_unit),
            condition=self.condition.dump(self.index, self.binding_unit),
            type=self.type_
        )
