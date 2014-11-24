

from __future__ import print_function  # To print without newline
import itertools
from string import Template
import sys
from extra import IndexDictionary, Fact
from static import UnarySet
import base
from compilation.helper import is_external


class Grounder(object):

    def __init__(self, task, compilation_index):
        self.task = task
        self.compilation_index = compilation_index
        self.type_info = {}
        self.init_index = {}
        self.init_facts = set()
        self.grounded_actions = {}
        self.all_grounded_actions = []
        self.goal_facts = []
        self.index_axioms()

    def ground(self, compilation_index):
        """ Grounds the task, adding some extra information to the compilation index on the way. """

        # Create an index of all possible state variables.
        compilation_index.variables = self.index_state_variables()

        # Index the symbol instantiations of the initial state
        # self.index_initial_state()

        # Ground axioms
        # self.ground_axioms()

        # Ground actions
        self.ground_actions()

        # Process goal
        self.process_goal()

        # expand the axioms in the actions predicates into their definitions.
        # self.expand_axioms()

    def flatten_ground_actions(self):
        self.all_grounded_actions = list(itertools.chain.from_iterable(self.grounded_actions.values()))

    def ground_actions(self):
        """ Ground a list of actions and return the resulting list of operators. """
        print("Grounding actions...")
        all_actions_counter = 0
        self.grounded_actions = {}
        for action in self.task.actions:
            self.grounded_actions[action.name], unpruned = self.ground_action(action)
            all_actions_counter += unpruned
        self.flatten_ground_actions()
        print("Num. actions: lifted / grounded (all) / grounded (pruned): {} / {} / {}".format(
            len(self.grounded_actions),
            all_actions_counter,
            len(self.all_grounded_actions)))

    # def expand_axioms(self):
    #     """ Expands all the axioms in the actions predicates and in the goal. """
    #     # Actions
    #     for a in self.all_grounded_actions:
    #         a.preconditions = self.process_axioms(a.preconditions)
    #
    #     #Goal
    #     self.processed_goal = self.process_axioms(self.processed_goal)
    #
    # def process_axioms(self, facts):
    #     expanded = Conjunction()
    #     for fact in facts:
    #         expanded.add(self.expand_fact(fact))
    #     return expanded.normalize()
    #
    # def expand_fact(self, fact):
    #     assert isinstance(fact, Fact)
    #     if fact.symbol in self.task.axiom_idx:
    #         fact = self.grounded_axioms[fact.var]
    #     return fact
    #
    # def ground_axioms(self):
    #     """ Ground the task axioms. """
    #     print("Grounding axioms...")
    #     self.grounded_axioms = {}
    #     for axiom in self.task.axioms:
    #         self.ground_axiom(axiom, self.grounded_axioms)
    #     print("Num. (grounded) axioms: {}".format(len(self.grounded_axioms)))
    #
    # @staticmethod
    # def get_preconditions(action):
    #     if isinstance(action.precondition, pddl.Conjunction):
    #         return action.precondition.parts
    #     elif isinstance(action.precondition, pddl.Atom):
    #         return [action.precondition]
    #     else:
    #         raise UnimplementedFeature('TODO - Precondition type not yet supported')
    #
    # def discard_individual_bindings(self, action, instantiations):
    #     """
    #      Filter out all those possible individual parameter assignments that involve static facts which are not
    #      reachable given the initial state.
    #      An example would be sokoban's move(?from, ?to, ?dir) action, with precondition move-dir(?from, ?to, ?dir),
    #      where move-dir is static. If there is no move-dir('pos-01-01', x, y) fact in the initial state, regardless
    #      of x and y, we can ensure that 'pos-01-01' is not a possible assignment for the action's first parameter.
    #      NOTE that here we still do not check for the combination of parameter instantiations, which will be done later.
    #      """
    #     for prec in self.get_preconditions(action):
    #         self.filter_unreachable_instantiations(prec, instantiations)
    #
    #     #if verbose_logging:
    #     #    logging.info('Static precondition analysis removed %d possible objects' % remove_debug)
    #
    # def filter_unreachable_instantiations(self, prec, instantiations):
    #     """ @see discard_individual_bindings"""
    #     if code.is_feasibility_atom(prec):
    #         return
    #
    #     # Currently not still handling functional expressions in preconditions,
    #     # although nothing would prevent us from doing so, in principle
    #     assert(isinstance(prec, (pddl.Atom, pddl.NegatedAtom)))
    #
    #     symbol = prec.predicate
    #
    #     if not symbol in self.statics:
    #         return
    #
    #     def valid(param_position, obj):
    #         init_state_instantiations = self.init_index[symbol][param_position]
    #
    #         if isinstance(prec, pddl.Atom):
    #             return obj in init_state_instantiations
    #         else:  # Negated precondition
    #             return obj not in init_state_instantiations
    #
    #     # For each parameter, we only keep those instantiations which actually make sense on the initial state,
    #     # i.e. those for which the initial state contains some fact.
    #     for param_pos, param in enumerate(prec.args):
    #         instantiations[param] = [o for o in instantiations[param] if valid(param_pos, o)]

    def compute_possible_action_instantiations(self, action):
        """ Return a dictionary mapping each action parameter to all its possible object instantiations. """
        return {p.name: self.task.type_map[p.typename] for p in action.parameters}

    def process_derived_symbols(self, action, binding):
        if action.has_defined_symbols():
            for symbol, routine in action.define_index.items():
                binding[symbol] = routine.execute(binding)

    def ground_action(self, action):
        """
        Ground the action and return the resulting list of operators.
        """
        print("Grounding action '{}'.".format(action), end='')
        param_to_objects = self.compute_possible_action_instantiations(action)

        # TODO - ACTIVATE ONCE WE DETECT THE STATIC ELEMENTS?
        # self.discard_individual_bindings(action, param_to_objects)

        # save a list of possible assignment tuples (param_name, object)
        domain_lists = [[(name, obj) for obj in objects] for name, objects in param_to_objects.items()]

        num_instantiations = 1
        for dom in domain_lists:
            num_instantiations *= len(dom)
        onepercent = int(num_instantiations / 100) if num_instantiations > 1000 else None

        print(" There are {} possible parameter instantiations...".format(num_instantiations), end='')
        sys.stdout.flush()

        # Iterate over all possible parameter bindings
        grounded = []
        for i, assign in enumerate(itertools.product(*domain_lists), 1):
            # Print 5%, 10%, 15%, ... progress indicators
            if onepercent is not None and i % onepercent == 0 and int(i / onepercent) % 5 == 0:
                print("{}%, ".format(int(i / onepercent)), end='')
                sys.stdout.flush()

            binding = dict(assign)

            # x = self.ground_preconditions(action, binding)
            # preconditions = self.process_preconditions(x)  # This performs static reachability analysis
            # if preconditions is None:  # Unapplicable binding
            #     continue
            # effects = [self.ground_effect(eff, binding) for eff in action.effects]

            # print("Grounding action {} with binding {}".format(action, binding))

            # Compute the derived symbols and add them to the binding
            self.process_derived_symbols(action, binding)

            grounded_action = base.GroundedAction.create_from_lifted_action(self.task, action, binding)
            if grounded_action is not None:  # i.e. if it is statically reachable
                grounded.append(grounded_action)

        print(" Done")
        return grounded, num_instantiations

    # def ground_axiom(self, axiom, grounded):
    #     """ Ground an axiom. """
    #     param_to_objects = self.compute_possible_action_instantiations(axiom)
    #
    #     # save a list of possible assignment tuples (param_name, object)
    #     domain_lists = [[(name, obj) for obj in objects] for name, objects in param_to_objects.items()]
    #
    #     # Iterate over all possible parameter bindings
    #     converter = FormulaConverter(self.task, self.compilation_index)
    #     for assign in itertools.product(*domain_lists):
    #         binding = dict(assign)
    #         var = StateVariable.create(axiom).ground(binding)
    #         grounded[var] = converter.ground_formula(axiom.condition, binding)

    def index_initial_state(self):
        """ For each symbol in the initial state, we index all the possible values (i.e. objects)
        that each of its arguments can take in this initial state.
        Thus, e.g. self.initial_state_index['clear'][2] contains a set with all the possible instantiations
        that the second argument of the 'clear' predicate can take on the initial state.
        """
        self.init_facts = set()
        for fact in self.task.init_facts:
            self.init_facts.add(fact)
            symbol, args = fact.symbol, fact.args

            # Currently we only index predicative facts, since we are not performing
            # static reachability analysis on functional symbols
            if fact.is_predicate():
                # TODO - This hasn't been refactored yet, as we still don't use predicates.
                if not symbol in self.init_index:
                    # initialize a list of as many empty sets as arguments has the atom
                    self.init_index[symbol] = [set() for _ in range(len(args))]

                for arg_pos, arg in enumerate(args):
                    self.init_index[symbol][arg_pos].add(arg)

    # @staticmethod
    # def ground_effect(effect, assignment):
    #     assert(isinstance(effect.condition, pddl.conditions.Truth))  # condeffects not yet supported
    #     assert(not effect.parameters)
    #     fact = Fact.create(effect.literal)
    #     ground_fact = fact.ground(assignment)
    #     return ground_fact
    #
    # @staticmethod
    # def check_equality_fact(fact):
    #     assert isinstance(fact, GroundFact)
    #     return fact.eval()
    #
    # def ground_preconditions(self, action, binding):
    #     """
    #     """
    #     return (Fact.create(prec).ground(binding) for prec in self.get_preconditions(action))
    #
    # def process_preconditions(self, preconditions):
    #     """
    #     Returns None if it has been determined that the action preconditions can never be satisfied, and thus there is
    #      no need to consider the binding.
    #     """
    #     grounded = set()
    #     for fact in preconditions:
    #         if isinstance(fact, GroundFact):
    #             if not self.check_equality_fact(fact):
    #                 return None
    #             continue
    #
    #         if fact.symbol in self.statics:
    #             if fact not in self.init_facts:
    #                 # We have a static fact in the precondition which is no true in the initial situation,
    #                 # will never be. We can thus disregard the current grounding
    #                 return None
    #             else:
    #                 # We have a static fact in the precondition which is already true in the initial situation.
    #                 # We don't need to consider it.
    #                 pass
    #         else:
    #             # A normal situation: a non-static fact.
    #             grounded.add(fact)
    #     return grounded

    def index_state_variables(self):
        task = self.task
        variables = IndexDictionary()

        for symbol in task.symbols.values():
            if not self.is_variable(symbol.name):
                continue
            instantiations = [self.task.type_map[t] for t in symbol.arguments]
            for instantiation in itertools.product(*instantiations):
                variables.index(base.Variable(symbol.name, instantiation))
        return variables

    def process_goal(self):
        # Substitute the occurrences of object names by their actual indexes.
        self.process_procedures_code(self.task.goal.applicability_procedures)

    def process_procedures_code(self, procedures):
        for procedure in procedures:
            self.process_procedure_code(procedure)

    def process_procedure_code(self, procedure):
        procedure.code = self.process_code(procedure.code)

    def process_code(self, code):
        return Template(code).substitute(self.compilation_index.objects.obj_to_idx)

    def index_axioms(self):
        task = self.task
        task.axiom_idx = {}
        for axiom in task.axioms:
            task.axiom_idx[axiom.name] = axiom

    def get_relevant_init_facts(self):
        """ Return a list of all the non-static facts in the initial state. """
        init = self.task.init
        assert isinstance(init, base.State)
        facts = []
        for inst in init.instantiations.values():
            # assert isinstance(inst, (base.PredicateInstantiation, base.FunctionInstantiation))
            if isinstance(inst, UnarySet):  # A predicate
                # symbol = self.task.domain.symbols[inst.symbol]
                # for point in self.compilation_index.objects.
                # for point in inst.set:
                #     facts.append(Fact(Variable(inst.symbol, tuple(point)), 1))
                for args in inst.elems:
                    # TODO - We should initialize the points not in the set to 0, although that'll be done
                    # TODO - by default by the compiler.
                    facts.append(Fact(base.Variable(inst.name, args), 1))
            else:  # A function
                for args, val in inst.elems.items():
                    facts.append(Fact(base.Variable(inst.name, args), val))
        return facts

    def is_variable(self, name):
        return not is_external(name) and name not in self.task.static_symbols and name not in self.task.axiom_idx

