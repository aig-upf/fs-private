"""
    Several method to create a FSTaskIndex from a task as given by FD's PDDL parser.
"""
from collections import OrderedDict
import itertools

from .. import utils
from . import pddl

from . import adl
from . import fstrips as fs
from . import pddl_helper
from . import static
from . import util
from .exceptions import ParseException
from .asp.problem import get_effect_symbols
from .util import IndexDictionary
from .fstrips_components import FSActionSchema, FSFormula, FSNamedFormula, FSMetric
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
    for func in adl_functions:
        if func.name != 'total-cost' and func.function_type != 'number':
            filtered.append(func)
    return filtered


def create_fs_task(fd_task, domain_name, instance_name, disable_static_analysis):
    """ Create a problem domain and instance and perform the appropriate validity checks """
    types, type_map, supertypes = process_problem_types(fd_task.types, fd_task.objects, fd_task.bounds)
    task = FSTaskIndex(domain_name, instance_name)
    task.process_objects(fd_task.objects)
    task.process_types(types, type_map, supertypes)
    task.process_symbols(actions=fd_task.actions, predicates=fd_task.predicates, functions=fd_task.functions,
                         no_static_symbols=disable_static_analysis)
    task.process_state_variables(create_all_possible_state_variables(task.symbols, task.static_symbols, type_map))
    task.process_initial_state(filter_out_action_cost_atoms(fd_task.init, task.action_cost_symbols))
    task.process_actions(fd_task.actions)
    task.process_processes([])
    task.process_events([])
    task.process_goal(fd_task.goal)
    task.process_state_constraints(fd_task.constraints)
    task.process_axioms(fd_task.axioms)
    task.process_metric(None)
    task.process_transitions(fd_task.transitions)
    return task


def create_fs_plus_task(fsp_task, domain_name, instance_name, disable_static_analysis):
    """ Create a problem domain and instance and perform the appropiate validty checks """
    types, type_map, supertypes = process_problem_types(fsp_task.types, fsp_task.objects, fsp_task.bounds)
    task = FSTaskIndex(domain_name, instance_name)
    print("Creating FS+ task: Processing objects...")
    task.process_objects(fsp_task.objects)
    print("Creating FS+ task: Processing types...")
    task.process_types(types, type_map, supertypes)
    # print("Types:", types)
    # print("Type -> Domain Map:", type_map)
    # MRJ: takes into account actions, events and processes
    print("Creating FS+ task: Processing symbols...")
    task.process_symbols(actions=fsp_task.actions, events=fsp_task.events,
                         processes=fsp_task.processes, constraints=fsp_task.constraint_schemata,
                         predicates=fsp_task.predicates, functions=fsp_task.functions, no_static_symbols=disable_static_analysis)
    print("Creating FS+ task: Processing state variables...")
    task.process_state_variables(create_all_possible_state_variables(task.symbols, task.static_symbols, type_map))
    print("Creating FS+ task: Processing initial state...")
    task.process_initial_state(filter_out_action_cost_atoms(fsp_task.init, task.action_cost_symbols))
    print("Creating FS+ task: Processing actions...")
    task.process_actions(fsp_task.actions)
    print("Creating FS+ task: Processing processes...")
    task.process_processes(fsp_task.processes)
    print("Creating FS+ task: Processing events...")
    task.process_events(fsp_task.events)
    print("Creating FS+ task: Processing the goal...")
    task.process_goal(fsp_task.goal)
    print("Creating FS+ task: Processing state constraints...")
    task.process_state_constraints(fsp_task.constraints)
    task.process_lifted_state_constraints(fsp_task.constraint_schemata)
    print("Creating FS+ task: Processing axioms...")
    task.process_axioms(fsp_task.axioms)
    print("Creating FS+ task: Processing metric...")
    task.process_metric(fsp_task.metric)
    return task


def create_fs_task_from_adl(adl_task, domain_name, instance_name):
    # MRJ: steps from fs_task.create_fs_task are copied here to act as both a reminder and a TODO list

    sorted_objs = [adl_task.objects[name] for name in adl_task.sorted_object_names]
    types, type_map, supertypes = process_problem_types(adl_task.types.values(), sorted_objs, [])
    task = FSTaskIndex(domain_name, instance_name)

    task.process_objects(sorted_objs)
    task.process_types(types, type_map, supertypes)

    adl_functions = filter_out_action_cost_functions(adl_task.functions.values())
    adl_predicates = adl_task.predicates.values()

    task.process_adl_symbols(adl_task.actions.values(), adl_predicates, adl_functions)

    simple_varlist = create_all_possible_state_variables(task.symbols, task.static_symbols, type_map)
    reach_pruned_varlist = create_all_possible_state_variables_from_groundings(adl_predicates, adl_functions,
                                                                         task.objects, task.static_symbols)

    undetected = [v for v in simple_varlist.objects if v not in set(reach_pruned_varlist.objects)]
    print("{} static atoms are going as state variables for ignoring reachability analysis".format(len(undetected)))
    # task.process_state_variables(reach_pruned_varlist)
    task.process_state_variables(simple_varlist)

    task.process_adl_initial_state(adl_task)
    task.process_processes([])
    task.process_events([])
    task.process_adl_actions(adl_task.actions, adl_task.sorted_action_names)
    task.process_adl_goal(adl_task)
    task.process_state_constraints([])  # No state constr. possible in ADL, but this needs to be invoked nevertheless
    task.process_axioms([])
    task.process_metric(None)
    task.process_transitions([])  # We simply initalize the transitions attribut with an empty set of transitions

    return task


def _process_fluent_atoms(fd_initial_fluent_atoms):
    initial_fluent_atoms = []
    for name, args, value in fd_initial_fluent_atoms:
        args = [int(a) if utils.is_int(a) else a for a in args]
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
        self.supertypes = util.UninitializedAttribute('supertypes')
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
        self.event_schemas = util.UninitializedAttribute('event_schemas')
        self.process_schemas = util.UninitializedAttribute('process_schemas')
        self.constraint_schemas = util.UninitializedAttribute('constraint_schemas')
        self.metric = util.UninitializedAttribute('metric')
        self.groundings = None

    def process_types(self, types, type_map, supertypes):
        # Each typename points to its (unique) 1-based index (index 0 is reserved for bools)
        self.types = {t: i for i, t in enumerate(types, 1)}
        self.type_map = type_map
        self.supertypes = supertypes

    def process_objects(self, objects):
        # Each object name points to it unique 0-based index / ID
        self.objects, self.object_types = self._index_objects(objects)

    def process_symbols(self, **kwargs):
        actions = kwargs.get('actions', [])
        events = kwargs.get('events', [])
        processes = kwargs.get('processes', [])
        constraints = kwargs.get('constraints', [])
        predicates = kwargs.get('predicates', [])
        functions = kwargs.get('functions', [])
        no_static_symbols = kwargs.get('no_static_symbols', False)

        self.symbols, self.symbol_types, self.action_cost_symbols = self._index_symbols(predicates, functions)
        self.symbol_index = {name: i for i, name in enumerate(self.symbols.keys())}

        self.all_symbols = list(self.symbol_types.keys())

        if no_static_symbols :
            self.fluent_symbols = self.all_symbols
            self.static_symbols = set("=") # only equality
            return

        # All symbols appearing on some action, process or event effect are fluent
        self.fluent_symbols = set(pddl_helper.get_effect_symbol(eff) for action in actions for eff in action.effects)
        self.fluent_symbols |= set(pddl_helper.get_effect_symbol(eff) for proc in processes for eff in proc.effects)
        self.fluent_symbols |= set(pddl_helper.get_effect_symbol(eff) for evt in events for eff in evt.effects)

        # MRJ: very unsatisfying fix
        for sym in self.all_symbols:
            if util.is_external(sym):  # symbol is procedurally defined
                self.fluent_symbols.add(sym)

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
            if s.name == 'total-cost':  # Ignore action costs
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
                    args = tuple(int(a) if utils.is_int(a) else a for a in atom.fluent.args)
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
        elif isinstance(expression, pddl.pddl_types.TypedObject):
            if expression.name not in self.objects:
                raise ParseException(
                    "Functions need to be instantiated to plain objects: symbol {} is not an object".format(
                        expression.symbol))
            return expression
        else:
            if expression.symbol not in self.objects:
                raise ParseException("Functions need to be instantiated to plain objects: "
                                     "symbol {} is not an object".format(expression.symbol))
            return expression.symbol

    def process_state_variables(self, state_variables):
        self.state_variables = state_variables

    def process_actions(self, actions):
        self.action_schemas = [FSActionSchema(self, action, "control") for action in actions]

    def process_processes(self, processes):
        self.process_schemas = [FSActionSchema(self, proc, "natural") for proc in processes]

    def process_events(self, events):
        self.event_schemas = [FSActionSchema(self, evt, "exogenous") for evt in events]

    def process_metric(self, metric):
        if metric is None:
            self.metric = FSMetric(self, None, None)
            return
        self.metric = FSMetric(self, metric.optimization, metric.expr)

    def process_adl_actions(self, actions, sorted_action_names):
        sorted_act = [actions[name] for name in sorted_action_names if name in actions]
        self.action_schemas = [FSActionSchema(self, adl.convert_adl_action(act)) for act in sorted_act]
        self.groundings = {act.name: act.groundings for act in sorted_act}

    def process_goal(self, goal):
        self.goal = FSFormula(self, goal)

    def process_adl_goal(self, adl_task):
        self.process_goal(adl.process_adl_flat_formula(adl_task.flat_ground_goal_preconditions))

    def process_state_constraints(self, constraints):
        self.state_constraints = [FSFormula(self, constraints)]

    def process_lifted_state_constraints(self, constraint_schemas):
        self.state_constraints += [FSNamedFormula(self, c.name, c.args, c.condition) for c in constraint_schemas]

    def process_axioms(self, axioms):
        """ An axiom is just a (possibly lifted) named formula. """
        self.axioms = [FSNamedFormula(self, axiom.name, axiom.parameters, axiom.condition) for axiom in axioms]

    def process_transitions(self, transitions):
        self.transitions = transitions


def _check_symbol_in_initial_state(s, symbols):  # A small helper
    if s == 'total-cost':  # We ignore the 'total-cost' initial specification
        return False

    if util.is_external(s):
        raise RuntimeError("The extension of an external symbol cannot ne specified in the initial state")

    if s not in symbols:
        raise ParseException("Unknown symbol: '{}'".format(s))

    return True
