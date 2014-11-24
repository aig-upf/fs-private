import argparse
import os
import subprocess
import sys
import shutil

import base
import ccode
import taskgen
import util


sys.path.append(os.path.abspath('..'))

from grounding import Grounder
from index import CompilationIndex
from templates import tplManager
from compilation.static import classify_symbols
from static import DataElement
import pddl  # This should be imported from a custom-set PYTHONPATH containing the path to Fast Downward's PDDL parser


def parse_arguments():
    parser = argparse.ArgumentParser(description='Parse a given problem instance from a given benchmark set.')
    parser.add_argument('--set', required=True, help="The name of the benchmark set.")
    parser.add_argument('--instance', required=True, help="The problem instance filename.")
    parser.add_argument('--translator', required=False, help="The directory containing the problem translation code.")
    parser.add_argument('--debug', action='store_true', help="Flag to compile in debug mode.")

    args = parser.parse_args()
    args.instance_dir = os.path.dirname(args.instance)
    return args


def import_translator(args, task):
    """ Import the translator modules specific to the domain from the given path """
    if args.translator is None:
        from generic_translator import Translator
        return Translator(task)

    old = sys.path
    sys.path = [args.translator]
    try:
        from translator import Translator  # This will be imported from the appended path.
    except ImportError:
        sys.path = old
        from generic_translator import Translator

    return Translator(task)


class ActionIndex(object):  # TODO - Migrate to IndexDictionary
    def __init__(self, action):
        # Index the position of each parameter
        self.param_idx = {param.name: i for i, param in enumerate(action.parameters, 0)}

        # index the action effect affected variables.
        self.index_affected_variables(action)

    def get_var_idx(self, var):
        assert False  # TODO
        return self.affected_vars_inv[var]

    def index_affected_variables(self, action):
        pass
        # for eff in action.effects:
        #     fact = Fact.create(eff.literal)
        #     self.affected_vars_inv[fact.var] = len(self.affected_vars)
        #     self.affected_vars.append(fact.var)


class TaskPreprocessor(object):
    def __init__(self, task):
        self.task = task

    def do(self):
        self.process_actions()
        # self.process_init()
        self.task.axioms = []  # TODO
        return self.task

    # def process_init(self):
        # facts = []
        # for symbol, inst in self.task.init.instantiations.items():
        #     assert isinstance(inst, (base.PredicateInstantiation, base.FunctionInstantiation))
        #     if isinstance(inst, base.PredicateInstantiation):
        #         for point in inst.set:
        #             facts.append(Fact(StateVariable(symbol, tuple(point)), value))
        #     else:
        #         for point, value in inst.mapping.items():
        #             facts.append(Fact(StateVariable(symbol, tuple(point)), value))
        #
        # self.task.init_facts = facts

    def process_actions(self):
        actions = []
        for a in self.task.domain.actions:
            a.info = ActionIndex(a)
            actions.append(a)
        self.task.actions = actions


def parse_pddl_task(domain, instance):
    domain_pddl = pddl.pddl_file.parse_pddl_file("domain", domain)
    task_pddl = pddl.pddl_file.parse_pddl_file("task", instance)
    task = pddl.tasks.Task.parse(domain_pddl, task_pddl)
    return task


def create_domain(name, translator):
    types = translator.get_types()
    symbols = translator.get_symbols()
    actions = translator.get_actions()
    # name = translator.get_domain_name()
    return taskgen.create_problem_domain(name, types, symbols, actions)


def create_instance(name, translator, domain):
    objects = translator.get_objects()
    goal = translator.get_goal()
    translator.process_initial_state()
    init = translator.get_initial_state()
    data = translator.get_static_data()
    constraints = translator.get_state_constraints()
    return taskgen.create_problem_instance(name, translator.task, domain, objects, init, goal, data, constraints)


def extract_names(instance, domain_filename):
    inst = os.path.splitext(os.path.basename(instance))[0]
    dom = os.path.basename(os.path.dirname(domain_filename))
    return inst, dom


def main():
    args = parse_arguments()
    domain_filename = pddl.pddl_file.extract_domain_name(args.instance)
    task = parse_pddl_task(domain_filename, args.instance)

    classify_symbols(task)
    task.index = CompilationIndex(task)

    translator = import_translator(args, task)
    instance_name, domain_name = extract_names(args.instance, domain_filename)
    domain = create_domain(domain_name, translator)
    instance = create_instance(instance_name, translator, domain)
    instance = TaskPreprocessor(instance).do()
    translate_pddl(instance=instance, setname=args.set, debug=args.debug, dirname=args.instance_dir)


def translate_pddl(instance, setname, debug, dirname):
    """
    """
    print("Parsing problem instance {}...".format(instance))

    domain = instance.domain
    translation_dir = domain.name + '/' + instance.name

    print("Translating and compiling problem instance {}...".format(instance.name))

    inst_name, translation_dir = translate_and_compile(instance, setname, dirname, translation_dir, debug)
    return inst_name, translation_dir


def compile_translation(translation_dir, debug=False, predstate=False):
    """
    Copies the relevant files from the BFS directory to the newly-created translation directory,
     and then calls scons to compile the problem there.
    """
    debug_flag = "debug={0}".format(1 if debug else 0)
    predstate_flag = "predstate=1" if predstate else ''

    bfs_dir = os.path.abspath('../planners/gbfs')
    shutil.copy(bfs_dir + '/main.cxx', translation_dir)
    shutil.copy(bfs_dir + '/SConstruct', translation_dir + '/SConstruct')

    command = "scons {} {}".format(debug_flag, predstate_flag)

    print("Executing '{0}' on directory '{1}'\n".format(command, translation_dir))
    sys.stdout.flush()  # Flush the output to avoid it mixing with the subprocess call.
    output = subprocess.call(command.split(), cwd=translation_dir)
    if not output == 0:
        raise RuntimeError('Error compiling problem at {0}'.format(translation_dir))


def translate_and_compile(instance, setname, dirname, translation_dir=None, debug=False):
    gen = Generator(instance, setname, translation_dir)
    translation_dir = gen.translate(dirname)
    compile_translation(translation_dir, debug, gen.task.domain.is_predicative())
    return gen.get_normalized_task_name(), translation_dir


class Generator(object):
    def __init__(self, instance, setname, translation_dir=None):
        self.task = instance
        self.index = CompilationIndex(instance)
        self.grounder = Grounder(instance, self.index)
        self.out_dir = ''
        self.symbol_decl = []
        self.symbol_init = []
        self.goal_code = None
        self.action_code = {}
        self.init_filenames(setname, translation_dir)

    def translate(self, dirname):
        self.preprocess_task()
        self.grounder.ground(self.index)
        self.process_elements()
        self._generate_generator_code()
        self._generate_components_code(dirname)

        self.dump_variable_data()
        self.dump_object_data()
        self.dump_action_data()
        self.dump_init_data()
        self.dump_constraints_data()
        self.dump_goal_data()

        print("Problem '{problem}' translated to directory '{dir}'".format(
            problem=self.task.get_complete_name(), dir=self.out_dir)
        )

        return self.out_dir

    def process_elements(self):
        self.action_code = {act.name: ccode.process_action(act) for act in self.task.actions}
        self.goal_code = ccode.process_goal(self.task.goal, self.index)

    def _generate_generator_code(self):
        # (No actual substitutions at the moment:)
        self.save_translation('generator.hxx', tplManager.get('generator.hxx').substitute())
        self.save_translation('generator.cxx', tplManager.get('generator.cxx').substitute())

    def _generate_components_code(self, dirname):
        # components.hxx:
        self.save_translation('components.hxx', tplManager.get('components.hxx').substitute(
            action_definitions=self.get_action_definitions(),
            method_factories=self.get_method_factories(),
            goal_evaluator_definition=self.get_goal_evaluator(),
            data_declarations=self.process_data_code('get_declaration'),
            data_accessors=self.process_data_code('get_accessor'),
            external_methods=self.load_external_methods(dirname)
        ))

        # components.cxx:
        self.save_translation('components.cxx', tplManager.get('components.cxx').substitute(
            action_code=self.get_actions_cxx_code(),
            data_initializations=self.process_data_code('get_initialization')
        ))

    def process_data_code(self, method):
        processed = [getattr(elem, method)(self.index.objects.obj_to_idx) for elem in self.task.static_data.values()
                     if isinstance(elem, DataElement)]
        return '\n\t'.join(processed)

    def get_normalized_task_name(self):
        return util.normalize(self.task.name)

    def get_normalized_domain_name(self):
        return util.normalize(self.task.domain.name)

    def init_filenames(self, setname, translation_dir=None):
        if translation_dir is None:
            dir_end = self.get_normalized_domain_name() + '/' + self.get_normalized_task_name() + '/'
        else:
            dir_end = translation_dir

        self.out_dir = os.path.abspath('../generated/' + setname + '/' + dir_end)

        if not os.path.isdir(self.out_dir):
            os.makedirs(self.out_dir)

    def get_actions_cxx_code(self):
        elems = []

        for action in self.task.actions:
            elems.append(tplManager.get('action_name_init').substitute(
                actionName=util.normalize_action_name(action.name),
                name=action.name,
            ))

            elems.append(tplManager.get('action_signature_init').substitute(
                actionName=util.normalize_action_name(action.name),
                signature=self._get_action_signature(action),
            ))

        return '\n'.join(elems)

    def get_method_factories(self):
        lines = (self.get_action_factory_line(a) for a in self.task.actions)
        return tplManager.get('method-factories').substitute(
            lines='\n\t\t'.join(lines)
        )

    def get_action_factory_line(self, action):
        return tplManager.get('action_factory_line').substitute(
            actionName=util.normalize_action_name(action.name),
        )

    def get_action_definitions(self):
        elems = []
        tpl = tplManager.get('action')

        for action in self.task.actions:
            action_code = self.action_code[action.name]

            elems.append(tpl.substitute(
                actionName=util.normalize_action_name(action.name),
                constructor=self._get_action_constructor(action),
                applicability_code_switch=action_code.applicability_code_switch,
                effect_code_switch=action_code.effect_code_switch,
            ))

        return '\n'.join(elems)

    def index_fact(self, fact):
        return '{idx}={val}'.format(
            idx=self.index.variables.get_index(fact.var),
            val=self.get_value_idx(fact.value))

    def dump_constraints_data(self):
        """ Saves the data related to state constraints """
        serialized = []
        for constraint in self.task.constraints:
            variable_idxs = self.gen_vector([self.index.variables.get_index(var) for var in constraint.args])
            serialized.append('#'.join([str(constraint), constraint.name, variable_idxs]))

        self.dump_data('constraints', serialized)

    def dump_init_data(self):
        """
        Saves the data related to the initial state:
         - The total number of state variables
         - The initial state facts themselves.
         """
        init_data = [str(len(self.index.variables))]  # The number of state variables

        facts = self.grounder.get_relevant_init_facts()
        indexed_facts = (self.index_fact(f) for f in facts)
        init_data.append(','.join(indexed_facts))
        self.dump_data('init', init_data)

    def gen_vector_of_vectors(self, data):
        return '/'.join(self.gen_vector(inner) for inner in data)

    def gen_vector(self, data):
        return ','.join(map(str, data))

    def dump_variable_data(self):
        data = []
        for i, var in enumerate(self.index.variables.idx_to_obj):
            data.append("{}.{}#{}".format(i, var, self.task.domain.symbol_types[var.symbol]))

        self.dump_data('variables', '\n'.join(data))

    def dump_object_data(self):
            self.dump_data('objects', self.index.objects.dump_index(print_index=False))

    def serialize_variables_data(self, procedures):
        """
        Serializes the indexes of the relevant and affected variables data of a set of procedures.
        For applicability procedures, the set of affected variables is empty.
        """
        rel_vars, aff_vars = [], []
        for proc in procedures:
            rel_vars.append([str(self.index.variables.get_index(var)) for var in proc.variables])
            if hasattr(proc, 'affected_variables'):
                aff_vars.append([str(self.index.variables.get_index(var)) for var in proc.affected_variables])
        return self.gen_vector_of_vectors(rel_vars), self.gen_vector_of_vectors(aff_vars)

    def dump_action_data(self):
        action_data = []
        names = []
        grounded_actions = self.grounder.grounded_actions
        idx = 0
        for lifted_action, grounded in grounded_actions.items():
            classname = util.normalize_action_name(lifted_action)
            for action in grounded:
                binding = self.gen_vector(self.index.objects[x] for x in action.binding)
                derived = self.gen_vector(self.index.objects[x] for x in action.derived)

                arv, _ = self.serialize_variables_data(action.applicability_procedures)
                erv, eav = self.serialize_variables_data(action.effect_procedures)

                # Format: a number of elements defining the action, separated by '#' signs, as follows:
                # (1) Action name
                # (2) classname
                # (3) binding
                # (4) derived objects
                # (5) applicability relevant vars
                # (6) effect relevant vars
                # (7) effect affected vars
                numbered_name = str(idx) + '.' + str(action)
                action_data.append('#'.join([numbered_name, classname, binding, derived, arv, erv, eav]))
                names.append(str(action))
                idx += 1

        self.dump_data('actions', action_data)
        self.dump_data('action-index', names)

    def dump_goal_data(self):
        assert isinstance(self.task.goal, base.Goal)
        goal_rel_vars, _ = self.serialize_variables_data(self.task.goal.applicability_procedures)
        self.dump_data('goal', goal_rel_vars)

    def get_goal_evaluator(self):
        return tplManager.get('goal').substitute(
            applicability_code_switch=self.goal_code.applicability_code_switch
        )

    def _get_all_symbol_declarations(self):
        return '\n\t'.join(self.symbol_decl)

    @staticmethod
    def _get_action_constructor(action):
        return tplManager.get('action-constructor').substitute(
            actionName=util.normalize_action_name(action.name),  # The normalized name
        )

    def _get_action_signature(self, action):
        types = [str(self.index.types[p.typename]) for p in action.parameters]
        return '{' + ','.join(types) + '}'

    def preprocess_task(self):
        # We substitute the predicate '=' for a syntactically acceptable predicate '_eq_'
        for pred in self.task.domain.get_predicates():
            if pred.name == '_eq_':
                raise RuntimeError('_eq_ is a reserved predicate name')
            pred.name = util.filter_extension_name(pred.name)

    def save_translation(self, name, translation):
        with open(self.out_dir + '/' + name, "w") as f:
            f.write(translation)

    def dump_data(self, name, data):
        if not isinstance(data, list):
            data = [data]

        basedir = self.out_dir + '/data'
        util.mkdirp(basedir)
        with open(basedir + '/' + name + '.data', "w") as f:
            for l in data:
                f.write(str(l) + '\n')

    def get_value_idx(self, value):
        """ Returns the appropriate integer index for the given value."""
        if isinstance(value, int):  # Variables of integer type are represented by the integer itself.
            return value
        else:
            # bool variables also need to be treated specially
            value = base.bool_string(value) if isinstance(value, bool) else value
            return self.index.objects[value]

    def load_external_methods(self, dirname):
        """ Loads externally defined procedures from file ext.hxx """
        c = util.load_file_safely(dirname + '/ext.hxx')
        return '' if c is None else c


if __name__ == "__main__":
    main()

