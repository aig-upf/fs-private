"""
    Several method to create a FSTaskIndex from a task as given by FD's PDDL parser.
"""
from collections import OrderedDict
import itertools

import python.utils
from . import pddl

from . import adl
from . import fstrips as fs
from . import pddl_helper
from . import static
from . import util
from .exceptions import ParseException
from .asp.problem import get_effect_symbols
from .util import IndexDictionary
from .fstrips_components import FSActionSchema, FSFormula, FSAxiom
from .object_types import process_problem_types
from .state_variables import create_all_possible_state_variables, create_all_possible_state_variables_from_groundings


def filter_out_action_cost_atoms(fd_initial_state, action_cost_symbols):
    filtered = []
    for atom in fd_initial_state:
        if isinstance(atom, pddl.Assign):
            name = atom.fluent.symbol
            if name in action_cost_symbols:
                continue
        filtered.append(atom)

    return filtered


def filter_out_action_cost_functions(adl_functions):
    filtered = []
    for function in adl_functions:
        if function.name != 'total-cost' and function.function_type != 'number':
            filtered.append(function)
    return filtered


def create_fs_task(fd_task, domain_name, instance_name):
    """ Create a problem domain and instance and perform the appropriate validity checks """
    types, type_map = process_problem_types(fd_task.types, fd_task.objects, fd_task.bounds)
    task = FSTaskIndex(domain_name, instance_name)
    task.process_objects(fd_task.objects)
    task.process_types(types, type_map)
    task.process_symbols(fd_task.actions, fd_task.predicates, fd_task.functions)
    task.process_state_variables(create_all_possible_state_variables(task.symbols, task.static_symbols, type_map))
    task.process_initial_state(filter_out_action_cost_atoms(fd_task.init, task.action_cost_symbols))
    task.process_actions(fd_task.actions)
    task.process_goal(fd_task.goal)
    task.process_state_constraints(fd_task.constraints)
    task.process_axioms(fd_task.axioms)
    return task


def create_fs_task_from_adl(adl_task, domain_name, instance_name):
    # MRJ: steps from fs_task.create_fs_task are copied here to act as both a reminder and a TODO list

    # types, type_map = process_problem_types(fd_task.types, fd_task.objects, fd_task.bounds)
    sorted_objs = [adl_task.objects[name] for name in adl_task.sorted_object_names]
    types, type_map = process_problem_types(adl_task.types.values(), sorted_objs, [])
    task = FSTaskIndex(domain_name, instance_name)

    task.process_objects(sorted_objs)
    task.process_types(types, type_map)

    adl_functions = filter_out_action_cost_functions(adl_task.functions.values())
    adl_predicates = adl_task.predicates.values()

    task.process_adl_symbols(adl_task.actions.values(), adl_predicates, adl_functions)

    state_var_list = create_all_possible_state_variables_from_groundings(adl_predicates, adl_functions,
                                                                         task.objects, task.static_symbols)
    task.process_state_variables(state_var_list)

    task.process_adl_initial_state(adl_task)
    task.process_adl_actions(adl_task.actions, adl_task.sorted_action_names)
    task.process_adl_goal(adl_task)
    task.process_state_constraints([])  # No state constr. possible in ADL, but this needs to be invoked nevertheless

    return task


def _process_fluent_atoms(fd_initial_fluent_atoms):
    initial_fluent_atoms = []
    for name, args, value in fd_initial_fluent_atoms:
        if value is None:  # A predicate
            # TODO - We should also initialize the non-specified points to 0? (done by the compiler anyway)
            initial_fluent_atoms.append(fs.Atom(fs.Variable(name, args), 1))  # '1' because it is non-negated
        else:  # A function
            initial_fluent_atoms.append(fs.Atom(fs.Variable(name, args), value))
    return initial_fluent_atoms


def _init_data_structures(symbols, fluent_symbols):
    init, static_data = {}, {}
    for s in symbols.values():
        if util.is_external(s.name):
            continue

        var = init if s.name in fluent_symbols else static_data
        var[s.name] = static.instantiate_extension(s)

    return init, static_data


class FSTaskIndex(object):
    def __init__(self, domain_name, instance_name):
        self.domain_name = domain_name
        self.instance_name = instance_name

        self.types = util.UninitializedAttribute('types')
        self.type_map = util.UninitializedAttribute('type_map')
        self.objects = util.UninitializedAttribute('objects')
        self.object_types = util.UninitializedAttribute('object_types')
        self.symbols = util.UninitializedAttribute('symbols')
        self.symbol_types = util.UninitializedAttribute('symbol_types')
        self.action_cost_symbols = util.UninitializedAttribute('action_cost_symbols')
        self.symbol_index = util.UninitializedAttribute('symbol_index')
        self.all_symbols = util.UninitializedAttribute('all_symbols')
        self.static_symbols = util.UninitializedAttribute('static_symbols')
        self.fluent_symbols = util.UninitializedAttribute('fluent_symbols')
        self.initial_fluent_atoms = util.UninitializedAttribute('initial_fluent_atoms')
        self.initial_static_data = util.UninitializedAttribute('initial_static_data')
        self.state_variables = util.UninitializedAttribute('state_variables')
        self.goal = util.UninitializedAttribute('goal')
        self.state_constraints = util.UninitializedAttribute('state_constraints')
        self.action_schemas = util.UninitializedAttribute('action_schemas')
        self.groundings = None
        self.axioms = []

    def process_types(self, types, type_map):
        # Each typename points to its (unique) 0-based index
        self.types = {t: i for i, t in enumerate(types)}
        self.type_map = type_map

    def process_objects(self, objects):
        # Each object name points to it unique 0-based index / ID
        self.objects, self.object_types = self._index_objects(objects)

    def process_symbols(self, actions, predicates, functions):
        self.symbols, self.symbol_types, self.action_cost_symbols = self._index_symbols(predicates, functions)
        self.symbol_index = {name: i for i, name in enumerate(self.symbols.keys())}

        self.all_symbols = list(self.symbol_types.keys())

        # All symbols appearing on some action effect are fluent
        self.fluent_symbols = set(pddl_helper.get_effect_symbol(eff) for action in actions for eff in action.effects)

        # The rest are static, including, by definition, the equality predicate
        self.static_symbols = set(s for s in self.all_symbols if s not in self.fluent_symbols) | set("=")

    def process_adl_symbols(self, actions, predicates, functions):
        predicates, functions = adl.convert_predicates_to_fd(predicates), adl.convert_functions_to_fd(functions)
        self.symbols, self.symbol_types, self.action_cost_symbols = self._index_symbols(predicates, functions)
        self.symbol_index = {name: i for i, name in enumerate(self.symbols.keys())}

        self.all_symbols = list(self.symbol_types.keys())

        # All symbols appearing on some action effect are fluent
        all_effects = itertools.chain.from_iterable(action.effects for action in actions)
        self.fluent_symbols = set(itertools.chain.from_iterable(get_effect_symbols(eff) for eff in all_effects))

        # The rest are static, including, by definition, the equality predicate
        self.static_symbols = set(s for s in self.all_symbols if s not in self.fluent_symbols) | set("=")

    def is_fluent(self, symbol_name):
        return symbol_name in self.fluent_symbols

    @staticmethod
    def _index_objects(objects):
        o_types = {}
        idx = IndexDictionary()
        idx.add(util.bool_string(False))  # 0
        idx.add(util.bool_string(True))  # 1
        # idx.add('undefined')  # Do we need an undefined object?
        for o in objects:
            idx.add(o.name)
            o_types[o.name] = o.type
        return idx, o_types

    @staticmethod
    def _index_symbols(predicates, functions):
        """
         This method takes care of analyzing any given task to determine which of the task symbols
         are fluent and which static.
        """
        symbols, symbol_types, action_cost_symbols = OrderedDict(), {}, set()

        for s in predicates:
            argtypes = [t.type for t in s.arguments]
            symbols[s.name] = fs.Predicate(s.name, argtypes)
            symbol_types[s.name] = 'bool'

        for s in functions:
            if s.name == 'total-cost' or s.type == 'number':  # Ignore action costs
                action_cost_symbols.add(s.name)
            else:
                argtypes = [t.type for t in s.arguments]
                symbols[s.name] = fs.Function(s.name, argtypes, s.type)
                symbol_types[s.name] = s.type

        return symbols, symbol_types, action_cost_symbols

    def process_initial_state(self, fd_initial_state):
        fd_initial_atoms = self._extract_initial_atom_names_and_arguments(fd_initial_state)

        # Distinguish fluent from static atoms, since we'll treat them differently
        fd_initial_fluent_atoms = [elem for elem in fd_initial_atoms if self.is_fluent(elem[0])]
        fd_initial_static_atoms = [elem for elem in fd_initial_atoms if not self.is_fluent(elem[0])]

        self.initial_fluent_atoms = _process_fluent_atoms(fd_initial_fluent_atoms)
        self.initial_static_data = self._process_static_atoms(fd_initial_static_atoms)

    def process_adl_initial_state(self, adl_task):
        initial_fluents = []
        for predicate in adl_task.predicates.values():
            for grounding in predicate.groundings:
                prop = (predicate, grounding)
                if prop in adl_task.initial_state:
                    initial_fluents.append((predicate.name, grounding, None))
        adl_initial_fluent_atoms = [elem for elem in initial_fluents if self.is_fluent(elem[0])]
        assert len(adl_initial_fluent_atoms) == len(initial_fluents)

        static_atoms = [(pred.name, args, None) for pred, args in adl_task.static_preds]
        self.initial_fluent_atoms = _process_fluent_atoms(adl_initial_fluent_atoms)
        self.initial_static_data = self._process_static_atoms(static_atoms)

    def _process_static_atoms(self, fd_initial_static_atoms):
        initial_static_data = {}
        for name, args, value in fd_initial_static_atoms:

            # In case the extension for this particular symbol has not yet been initialized
            if name not in initial_static_data:
                initial_static_data[name] = static.instantiate_extension(self.symbols[name])
            initial_static_data[name].add(args, value)

        return initial_static_data

    def _extract_initial_atom_names_and_arguments(self, fd_initial_state):
        names = []
        for atom in fd_initial_state:
            if isinstance(atom, pddl.Assign):
                name = atom.fluent.symbol
                if _check_symbol_in_initial_state(name, self.symbols):
                    args = tuple(int(a) if python.utils.is_int(a) else a for a in atom.fluent.args)
                    value = self.parse_value(atom.expression)
                    names.append((name, args, value))

            elif isinstance(atom, pddl.Atom):
                if atom.negated:
                    raise RuntimeError("No negations allowed in the initialization of atoms")

                name = atom.predicate
                if _check_symbol_in_initial_state(name, self.symbols):
                    names.append((name, atom.args, None))

            else:
                raise RuntimeError("Unrecognized type of atom '{}'".format(atom))

        return names

    def parse_value(self, expression):
        if isinstance(expression, pddl.f_expression.NumericConstant):
            return expression.value
        else:
            if expression.symbol not in self.objects:
                raise ParseException("Functions need to be instantiated to plain objects: symbol {} is not an object".format(expression.symbol))
            return expression.symbol

    def process_state_variables(self, state_variables):
        self.state_variables = state_variables

    def process_actions(self, actions):
        self.action_schemas = [FSActionSchema(self, action) for action in actions]

    def process_adl_actions(self, actions, sorted_action_names):
        sorted_act = [actions[name] for name in sorted_action_names if name in actions]
        self.action_schemas = [FSActionSchema(self, adl.convert_adl_action(act)) for act in sorted_act]
        self.groundings = {act.name: act.groundings for act in sorted_act}

    def process_goal(self, goal):
        self.goal = FSFormula(self, goal)

    def process_adl_goal(self, adl_task):
        self.process_goal(adl.process_adl_flat_formula(adl_task.flat_ground_goal_preconditions))

    def process_state_constraints(self, constraints):
        self.state_constraints = FSFormula(self, constraints)

    def process_axioms(self, axioms):
        """ An axiom is just a (possibly lifted) named formula. """
        self.axioms = [FSAxiom(self, axiom) for axiom in axioms]


def _check_symbol_in_initial_state(s, symbols):  # A small helper
    if s == 'total-cost':  # We ignore the 'total-cost' initial specification
        return False

    if util.is_external(s):
        raise RuntimeError("The extension of an external symbol cannot ne specified in the initial state")

    if s not in symbols:
        raise ParseException("Unknown symbol: '{}'".format(s))

    return True
