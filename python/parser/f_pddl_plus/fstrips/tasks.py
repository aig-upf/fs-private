import sys
from ...pddl import actions
from ...pddl import axioms
from ...pddl import conditions
from ...pddl import predicates
from ...pddl import pddl_types
from ...pddl import functions
from ...pddl import f_expression
from ...pddl import metrics
from ...pddl import effects
import itertools
from antlr4 import *
from .lexer import FunctionalStripsLexer
from .parser import FunctionalStripsParser
from .fs_task_loader import FunctionalStripsLoader

def parse_file( name ) :
    input = FileStream(name)

    lexer = FunctionalStripsLexer(input)

    stream = CommonTokenStream(lexer)

    parser = FunctionalStripsParser(stream)

    tree = parser.pddlDoc()

    return stream, tree

class Task(object):
    def __init__(self, domain_name, task_name, requirements,
                 types, objects, predicates, functions, init, goal, actions, events, processes, axioms, constraints, constraint_schemata, gconstraints, bounds, metric):
        self.domain_name = domain_name
        self.task_name = task_name
        self.requirements = requirements
        self.types = types
        self.types_table = { type.name : type for type in self.types }
        self.objects = objects
        self.predicates = predicates
        self.functions = functions
        self.functions_table = { f.name : f for f in self.functions}
        self.init = init
        self.goal = goal
        self.actions = actions
        self.events = events
        self.processes = processes
        self.axioms = axioms
        self.axiom_counter = 0
        self.constraints = constraints
        self.constraint_schemata = constraint_schemata
        self.gconstraints = gconstraints
        self.bounds = bounds
        self.metric = metric

    def is_subtype( self, type_a, type_b ) :
        for type in self.types :
            if type_a == type.name :
                if type_b in type.supertype_names :
                    return True
                return False
        raise RuntimeError("Asked whether type '{}' is a subtype of '{}' but '{}' is not a know type".format( type_a, type_b, type_a ))

    def get_primitive_type( self, typename ) :
        qtype_supertypes = set()
        type = self.types_table[typename]
        while type.basetype_name is not None :
            type = self.types_table[ type.basetype_name ]

        return type.name


    def add_axiom(self, parameters, condition):
        name = "new-axiom@%d" % self.axiom_counter
        self.axiom_counter += 1
        axiom = axioms.Axiom(name, parameters, len(parameters), condition)
        self.predicates.append(predicates.Predicate(name, parameters))
        self.axioms.append(axiom)
        return axiom

    @staticmethod
    def parse(domain_pddl, task_pddl):
        print("Parsing domain description...")
        domain_stream, domain_parse_tree = parse_file( domain_pddl )
        print("Parsing problem description...")
        instance_stream, instance_parse_tree = parse_file( task_pddl )

        loader = FunctionalStripsLoader()
        loader.initialise()

        print("Processing domain AST...")
        loader.tokenStream = domain_stream
        loader.visit( domain_parse_tree )
        print("Processing instance AST...")
        loader.tokenStream = instance_stream
        loader.visit( instance_parse_tree )
        print("Post-Processing...")
        loader.doPostProcessing()

        assert loader.domain_name == loader.task_domain_name

        requirements = Requirements(sorted(set(loader.requirements)))

        objects = loader.constants + loader.objects

        # We temporarily comment out this. Probably should only be added when the :equality requirement is there.
        #init += [conditions.Atom("=", (obj.name, obj.name)) for obj in objects]

        # Currently unsupported stuff
        axioms = []
        gconstraints = []

        if len(loader.processes) > 0 :
            print( "Domain specifies continuous change...")
            print( "Setting up 'clock_time' state variable" )

            total_time = functions.TypedFunction( 'clock_time', [], 'number' )
            loader.functions.append(total_time)
            lhs_init_clock = f_expression.FunctionalTerm('clock_time', [])
            loader.init.append( f_expression.Assign( lhs_init_clock, f_expression.NumericConstant(0.0) ) )

            print("Setting up 'second_law_thermodynamics' process")
            second_law_rhs = f_expression.FunctionalTerm('+', [lhs_init_clock, f_expression.NumericConstant(1.0)])
            second_law_effect = effects.Effect([], conditions.Truth(), effects.AssignmentEffect(lhs_init_clock, second_law_rhs))
            second_law = actions.Action( '2nd_law_thermodynamics', [], 0, conditions.Truth(), [second_law_effect], None )
            loader.processes.append(second_law)


        print( "Checking metric expression is a grounded functional term...")
        if loader.metric is not None :
            if loader.metric.terminal_cost is not None :
                check_is_grounded_functional_term(loader.metric.terminal_cost)
            if loader.metric.stage_cost is not None :
                check_is_grounded_functional_term(loader.metric.stage_cost)

        print( "Checking the goal is a ground formula")
        if loader.goal is not None :
            for g in loader.goal.parts :
                if not g.is_ground() :
                    raise SystemExit( "Goal is not a ground formula!")

        return Task( loader.domain_name, loader.task_name, requirements, loader.types, objects,
                    loader.predicates, loader.functions, loader.init, loader.goal,
                    loader.actions, loader.events, loader.processes,
                    axioms, loader.constraints, loader.constraint_schemata,
                    gconstraints, loader.type_bounds, loader.metric)

    def dump(self, redirect=True):
        import sys
        if redirect :
            current_stdout = sys.stdout
            sys.stdout = open( 'problem.txt', 'w')
        print("Problem %s: %s [%s]" % (
            self.domain_name, self.task_name, self.requirements))
        print("Types:")
        for type in self.types:
            print("  %s" % type)
        print("Objects:")
        for obj in self.objects:
            print("  %s" % obj)
        print("Predicates:")
        for pred in self.predicates:
            print("  %s" % pred)
        print("Functions:")
        for func in self.functions:
            print("  %s" % func)
        print("Init:")
        for fact in self.init:
            print("  %s" % fact)
        print("Goal:")
        self.goal.dump()
        print("Actions:")
        for action in self.actions:
            action.dump()
        print("Events:")
        for event in self.events :
            event.dump()
        print("Processes:")
        for proc in self.processes :
            proc.dump()
        print("State constraints (ground):")
        if self.constraints is not None :
            self.constraints.dump()
        print("State constraints (schemata):")
        for constraint in self.constraint_schemata :
            constraint.dump()

        if self.axioms:
            print("Axioms:")
            for axiom in self.axioms:
                axiom.dump()

        print("Bounds:")
        for b in self.bounds :
            print( "{}".format(b))
        print("Metric:")
        print( "{}".format(self.metric) )
        if redirect :
            sys.stdout = current_stdout

class Requirements(object):
    def __init__(self, requirements):
        self.requirements = requirements
        for req in requirements:
            assert req in (
              ":strips", ":adl", ":typing", ":negation", ":equality",
              ":negative-preconditions", ":disjunctive-preconditions",
              ":existential-preconditions", ":universal-preconditions",
              ":quantified-preconditions", ":conditional-effects", ":object-fluents",
              ":derived-predicates", ":action-costs"), req
    def __str__(self):
        return ", ".join(self.requirements)

def check_is_grounded_functional_term( term ) :
    for argument in term.args :
        #print(argument)
        if isinstance( argument, f_expression.FunctionalTerm ) :
            check_is_grounded_functional_term( argument )
        elif isinstance( argument, pddl_types.TypedObject ) :
            if argument.name[0] == "?" : # it is a variable
                raise SystemExit("Term {} is not grounded".format(term))
        elif isinstance( argument, str ) :
            if argument[0] == '?' : # it is a variable
                raise SystemExit("Term {} is not grounded".format(term))

def check_atom_consistency(atom, same_truth_value, other_truth_value, atom_is_true=True):
    if atom in other_truth_value:
        raise SystemExit("Error in initial state specification\n" +
                         "Reason: %s is true and false." %  atom)
    if atom in same_truth_value:
        if not atom_is_true:
            atom = atom.negate()
        print("Warning: %s is specified twice in initial state specification" % atom)


def check_for_duplicates(elements, errmsg, finalmsg):
    seen = set()
    errors = []
    for element in elements:
        if element in seen:
            errors.append(errmsg % element)
        else:
            seen.add(element)
    if errors:
        raise SystemExit("\n".join(errors) + "\n" + finalmsg)
