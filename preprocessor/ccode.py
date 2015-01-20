from pddl import Atom, NegatedAtom

import base
import util
from templates import tplManager


def gen_param_unwrapping(parameters):
    elems = []
    for idx, param in enumerate(parameters, 0):
        unwrap = tplManager.get('binding_unwrap').substitute(varname=util.make_param_name(param.name), varidx=idx)
        elems.append(unwrap)

    return '\n\t\t'.join(elems)


def is_feasibility_atom(atom):
    """ Return true iff this atom is an equality predicate on two objects and can thus be statically checked
        during the feasibility analysis phase.
    """
    return is_equality_atom(atom) and all(util.is_action_parameter(arg) for arg in atom.args)


def is_comparison_atom(atom):
    """ Return true iff this atom is a flat equality comparison atom.
    """
    return is_equality_atom(atom) and all(isinstance(arg, str) for arg in atom.args)


def get_object_indirect_point(args):
    """ Computes the point on which a predicate is evaluated from a set of domain objects,
        getting their ID indirectly through a symbol table """
    return '{' + ','.join([get_object_id_extraction(x) for x in args]) + '}'


def get_object_id_extraction(obj_name):
    return tplManager.get('obj_id_extraction').substitute(name=obj_name)


def process_goal(goal, index):
    """ Process a given goal """
    code = GoalCode(goal, index)
    code.process()
    return code


def is_equality_atom(atom):
    return isinstance(atom, (Atom, NegatedAtom)) and atom.predicate == '='


def gen_extension_extraction(ext):
    """ Extracts an extension object from a state"""
    return tplManager.get('symbol_extraction').substitute(
        ext_name=util.make_extension_varname(ext.name),
        ext_idx=util.make_extension_idxname(ext.name),
    )


def process_action(action):
    """ Process a given action to extract all the relevant information for the code generation """
    assert isinstance(action, base.Action)
    code = ActionCode(action)
    code.process()
    return code


class ProblemObject(object):
    def __init__(self, name, val):
        self.name = name
        self.val = val


# A simple wrapper to signal values that we want _not_ to be processed.
class RawValue:
    def __init__(self, val):
        self.val = val


class TFact(object):
    """ Wrapper to print Fact objects """
    BOOL_VALUES = {True: 'true', False: 'false'}

    def __init__(self, fact):
        self.fact = fact

    def normalize_value(self):
        if isinstance(self.fact.value, bool):
            return self.BOOL_VALUES[self.fact.value]
        else:
            return util.make_param_or_object_name(self.fact.value)

    def print_initialization(self):
        return tplManager.get('fact_init').substitute(
            ext=util.make_extension_idxname(self.fact.symbol),
            point=self.fact.args.val if isinstance(self.fact.args, RawValue) else util.get_point(self.fact.args),
            value=self.normalize_value()
        )


class Procedure(object):
    def __init__(self):
        self.relevant_variables = []
        self.satisfaction_checks = []

    def add_relevant_var(self, var):
        self.relevant_variables.append(var)
        return self

    def add_satisfaction_check(self, value):
        assert(isinstance(value, (ProblemObject, RawValue)))  # So far we only accept value-like checks
        self.satisfaction_checks.append(value)

    def generate_relevance_code(self):
        return '\n\t'.join(self.generate_single_relevance_code(var) for var in self.relevant_variables)

    def generate_single_relevance_code(self, var):
        return tplManager.get('signature_construction').substitute(state_var=var.print_initialization())

    def generate_satisfaction_code(self):
        return '\n\t'.join(self.generate_single_satisfaction_code(var) for var in self.satisfaction_checks)

    def generate_single_satisfaction_code(self, var):
        val = var.val
        comment = '' if isinstance(var, RawValue) else "// '{}'".format(var.name)
        return tplManager.get('satisfaction_check').substitute(value=val, comment=comment)


class ApplicableObject(object):
    def __init__(self, procedures):
        self.procedures = procedures
    #
    # def translate_object(self, obj):
    #     return obj

    def get_procedures_code(self, procedure_list, symbol_map={}):
        return [proc.get_annotated_code(symbol_map) for proc in procedure_list]

    def process_applicability(self):
        # Applicability procedures
        blocks = self.get_procedures_code(self.procedures)
        self.applicability_code_switch = generate_switch_code('procedureIdx', blocks)
        self.applicability_constraints, self.constraint_instantiations = generate_constraint_code(blocks)


class ActionCode(ApplicableObject):
    def __init__(self, action):
        super(ActionCode, self).__init__(action.applicability_procedures)
        self.action = action
        self.effect_procedures = action.effect_procedures

    def process(self):
        self.process_applicability()
        self.process_effect_procedures()

    def process_effect_procedures(self):
        blocks = self.get_procedures_code(self.effect_procedures, self.action.parameter_map)
        self.effect_code_switch = generate_switch_code('procedureIdx', blocks,
                                                       default='throw std::runtime_error("Wrong procedure index");')


class GoalCode(ApplicableObject):
    def __init__(self, goal, index):
        super(GoalCode, self).__init__(goal.applicability_procedures)
        self.goal = goal
        self.index = index

    def process(self):
        self.process_applicability()
    #
    # def translate_object(self, obj):
    #     return self.index.objects[obj]


def generate_switch_code(switchvar, code_blocks, default=''):
    blocks = [generate_switch_case_code(i, block) for i, block in enumerate(code_blocks)]
    cases = '\t\t\t' + '\n\t\t\t'.join(blocks)
    return tplManager.get('switch_code').substitute(
        switchvar=switchvar,
        cases=cases,
        default=default
    )


def generate_constraint_code(blocks):
    classes, instantiations = [], []
    for i, code in enumerate(blocks, 0):
        classname = 'GoalConstraint{}'.format(i)
        classes.append(tplManager.get('constraint').substitute(
            classname=classname,
            code='\n\t\t'.join(code)
        ))
        instantiations.append(tplManager.get('constraint_instantiation').substitute(
            classname=classname,
            i=i
        ))
    return classes, instantiations


def generate_switch_case_code(num, code):
    indented_code = '\t\t\t\t' + '\n\t\t\t\t'.join(code)
    return tplManager.get('switch_case_code').substitute(i=num, code=indented_code)
