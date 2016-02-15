import argparse
import glob
import json
import operator
import os
import shutil
import subprocess
import sys
import taskgen
import util
from compilation.helper import is_external
from generic_translator import Translator

util.fix_seed()

sys.path.append(os.path.abspath('..'))

from grounding import Grounder
from templates import tplManager
from static import DataElement
import pddl  # This should be imported from a custom-set PYTHONPATH containing the path to Fast Downward's PDDL parser


def parse_arguments():
    parser = argparse.ArgumentParser(description='Parse a given problem instance from a given benchmark set.')
    parser.add_argument('--tag', required=True, help="The name of the generation tag.")
    parser.add_argument('--instance', required=True,
                        help="The problem instance filename (heuristics are used to determine domain filename).")
    parser.add_argument('--domain', required=False, help="The problem domain filename.", default=None)
    parser.add_argument('--planner', default="generic", help="The directory containing the planner sources.")
    parser.add_argument('--output_base', default="../generated",
                        help="The base for the output directory where the compiled planner will be left. "
                             "Additional subdirectories will be created with the name of the domain and the instance")
    parser.add_argument('--output', help="The final directory (without any added subdirectories)"
                                         " where the compiled planner will be left.")
    parser.add_argument('--debug', action='store_true', help="Flag to compile in debug mode.")
    parser.add_argument('--edebug', action='store_true', help="Flag to compile in extreme debug mode.")

    args = parser.parse_args()
    args.instance_dir = os.path.dirname(args.instance)
    return args


def parse_pddl_task(domain, instance):
    domain_pddl = pddl.pddl_file.parse_pddl_file("domain", domain)
    task_pddl = pddl.pddl_file.parse_pddl_file("task", instance)
    task = pddl.tasks.Task.parse(domain_pddl, task_pddl)
    return task


def create_instance(name, translator, domain):
    objects = translator.get_objects()
    init = translator.process_initial_state(domain.symbols)
    data = translator.get_static_data()
    instance = taskgen.create_problem_instance(name, translator.task, domain, objects, init, data)
    instance.goal_formula = translator.get_goal_formula()
    instance.state_constraints = translator.get_state_constraints()
    return instance


def extract_names(domain_filename, instance_filename):
    """ Extract the canonical domain and instance names from the corresponding filenames """
    domain = os.path.basename(os.path.dirname(domain_filename))
    instance = os.path.splitext(os.path.basename(instance_filename))[0]
    return domain, instance


def main():
    args = parse_arguments()
    if args.domain is None:
        args.domain = pddl.pddl_file.extract_domain_name(args.instance)
    task = parse_pddl_task(args.domain, args.instance)
    domain_name, instance_name = extract_names(args.domain, args.instance)

    domain = taskgen.create_problem_domain(task, domain_name)
    instance = create_instance(instance_name, Translator(task), domain)
    _, trans_dir = translate_pddl(instance, args)


def translate_pddl(instance, args):
    """
    """
    print("{0:<30}{1}".format("Problem instance:", instance.name))
    print("{0:<30}{1}".format("Chosen Planner:", args.planner))

    domain = instance.domain
    translation_dir = domain.name + '/' + instance.name
    inst_name, translation_dir = translate_and_compile(instance, translation_dir, args)
    return inst_name, translation_dir


def translate_and_compile(instance, translation_dir, args):
    gen = Generator(instance, args, translation_dir)
    translation_dir = gen.translate()
    move_files_around(args.instance_dir, args.instance, args.domain, translation_dir)
    compile_translation(translation_dir, args)
    return gen.get_normalized_task_name(), translation_dir


def compile_translation(translation_dir, args):
    """
    Copies the relevant files from the BFS directory to the newly-created translation directory,
     and then calls scons to compile the problem there.
    """
    debug_flag = "edebug=1" if args.edebug else ("debug=1" if args.debug else "")

    planner_dir = os.path.abspath(os.path.join('../planners', args.planner))

    shutil.copy(os.path.join(planner_dir, 'main.cxx'), translation_dir)
    shutil.copy(os.path.join(planner_dir, 'default.config.json'), os.path.join(translation_dir, 'config.json'))
    shutil.copy(os.path.join(planner_dir, 'SConstruct'), os.path.join( translation_dir, 'SConstruct'))

    command = "scons {}".format(debug_flag)

    print("{0:<30}{1}\n".format("Compilation command:", command))
    sys.stdout.flush()  # Flush the output to avoid it mixing with the subprocess call.
    output = subprocess.call(command.split(), cwd=translation_dir)
    if not output == 0:
        raise RuntimeError('Error compiling problem at {0}'.format(translation_dir))


def move_files_around(base_dir, instance, domain, target_dir):
    """ Moves the domain and instance description files plus additional data files to the translation directory """
    definition_dir = target_dir + '/definition'
    data_dir = target_dir + '/data'

    # Copy the domain and instance file to the subfolder "definition" on the destination dir
    util.mkdirp(definition_dir)
    shutil.copy(instance, definition_dir)
    shutil.copy(domain, definition_dir)

    # The ad-hoc external definitions file - if it does not exist, we use the default.
    if os.path.isfile(base_dir + '/external.hxx'):
        shutil.copy(base_dir + '/external.hxx', target_dir)

        if os.path.isfile(base_dir + '/external.cxx'):  # We also copy a possible cxx implementation file
            shutil.copy(base_dir + '/external.cxx', target_dir)

    else:
        default = tplManager.get('external_default.hxx').substitute()  # No substitutions for the default template
        util.save_file(target_dir + '/external.hxx', default)

    # Copy, if they exist, all data files
    origin_data_dir = base_dir + '/data'
    if os.path.isdir(origin_data_dir):
        for filename in glob.glob(os.path.join(origin_data_dir, '*')):
            if os.path.isfile(filename):
                shutil.copy(filename, data_dir)


class Generator(object):
    def __init__(self, instance, args, translation_dir=None):
        self.task = instance
        # self.index = CompilationIndex(instance)
        self.index = instance.task.index
        self.grounder = Grounder(instance, self.index)
        self.out_dir = ''
        self.init_filenames(args, translation_dir)
        self.symbol_index = {}

    def translate(self):
        self.preprocess_task()
        self.grounder.ground_state_variables(self.index)
        self.generate_components_code()
        self.symbol_index = {name: i for i, name in enumerate(self.task.symbols.keys())}

        data = {'variables': self.dump_variable_data(),
                'objects': self.dump_object_data(),
                'types': self.dump_type_data(),
                'action_schemata': self.task.domain.schemata,
                'state_constraints': self.task.state_constraints,
                'init': self.dump_init_data(),
                'goal': self.task.goal_formula,
                'functions': self.dump_function_data(),
                'problem': {'domain': self.task.domain.name, 'instance': self.task.name}
                }

        print("{0:<30}{1}".format("Translation directory:", self.out_dir))

        self.dump_data('problem', json.dumps(data), ext='json')

        return self.out_dir

    def generate_components_code(self):
        # components.hxx:
        self.save_translation('components.hxx', tplManager.get('components.hxx').substitute(
            method_factories=self.get_method_factories(),
        ))

        # components.cxx:
        self.save_translation('components.cxx', tplManager.get('components.cxx').substitute())

        # external_base.hxx:
        self.save_translation('external_base.hxx', tplManager.get('external_base.hxx').substitute(
            data_declarations=self.process_data_code('get_declaration'),
            data_accessors=self.process_data_code('get_accessor'),
            data_initialization=self.get_external_data_initializer_list()
        ))

        self.serialize_external_data()

    def process_data_code(self, method):
        processed = [getattr(elem, method)(self.index.objects.obj_to_idx) for elem in self.task.static_data.values()
                     if isinstance(elem, DataElement)]
        return '\n\t'.join(processed)

    def get_external_data_initializer_list(self):
        elems = []
        for elem in self.task.static_data.values():
            if isinstance(elem, DataElement):
                elems.append(elem.initializer_list())
            else:
                raise RuntimeError()
        if not elems:
            return ''
        return ': {}'.format(','.join(elems))

    def serialize_external_data(self):
        for elem in self.task.static_data.values():
            assert isinstance(elem, DataElement)
            serialized = elem.serialize_data(self.index.objects.obj_to_idx)
            self.dump_data(elem.name, serialized)

    def get_normalized_task_name(self):
        return util.normalize(self.task.name)

    def get_normalized_domain_name(self):
        return util.normalize(self.task.domain.name)

    def init_filenames(self, args, translation_dir=None):
        if translation_dir is None:
            dir_end = self.get_normalized_domain_name() + '/' + self.get_normalized_task_name() + '/'
        else:
            dir_end = translation_dir

        o_dir = args.output if args.output is not None else (args.output_base + '/' + args.tag + '/' + dir_end)
        self.out_dir = os.path.abspath(o_dir)

        if not os.path.isdir(self.out_dir):
            os.makedirs(self.out_dir)

    def get_method_factories(self):
        return tplManager.get('factories').substitute(
            functions=',\n\t\t\t'.join(self.get_function_instantiations()),
        )

    def dump_init_data(self):
        """ Saves the initial values of all state variables explicitly mentioned in the initialization """
        atoms = self.grounder.get_relevant_init_facts()
        indexer = lambda f: [self.index.variables.get_index(f.var), self.get_value_idx(f.value)]
        indexed_atoms = [indexer(f) for f in atoms]
        return dict(variables=len(self.index.variables), atoms=indexed_atoms)

    def dump_variable_data(self):
        res = []
        for i, var in enumerate(self.index.variables.idx_to_obj):
            data = self.dump_state_variable(var)
            res.append({'id': i, 'name': str(var), 'type': self.task.domain.symbol_types[var.symbol], 'data': data})
        return res

    def dump_state_variable(self, var):
        head = self.symbol_index[var.symbol]
        constants = [arg if util.is_int(arg) else self.index.objects.get_index(arg) for arg in var.args]
        return [head, constants]

    def dump_object_data(self):
        return [{'id': i, 'name': obj} for i, obj in enumerate(self.index.objects.dump_index(print_index=False))]

    def dump_function_data(self):
        res = []
        for name, symbol in self.task.symbols.items():
            i = self.symbol_index[name]

            # Collect the list of variables that arise from this particular symbol
            f_variables = [(i, str(v)) for i, v in enumerate(self.index.variables.idx_to_obj) if v.symbol == name]

            static = name in self.task.static_symbols

            # Store the function info as a tuple:
            # <function_id, function_name, <function_domain>, function_codomain, state_variables>
            res.append([i, name, symbol.arguments, symbol.codomain, f_variables, static])
        return res

    def dump_type_data(self):
        """ Dumps a map of types to corresponding objects"""
        data = []

        type_map = self.task.type_map
        _sorted = sorted(self.index.types.items(), key=operator.itemgetter(1))  # all types, sorted by type ID
        for t, i in _sorted:
            objects = type_map[t]
            if objects and isinstance(objects[0], int):  # We have a bounded int variable
                data.append([i, t, 'int', [objects[0], objects[-1]]])
            else:
                object_idxs = [str(self.index.objects.get_index(o)) for o in objects]
                data.append([i, t, object_idxs])

        return data

    def preprocess_task(self):
        # We substitute the predicate '=' for a syntactically acceptable predicate '_eq_'
        for pred in self.task.domain.get_predicates():
            if pred.name == '_eq_':
                raise RuntimeError('_eq_ is a reserved predicate name')
            pred.name = util.filter_extension_name(pred.name)

    def save_translation(self, name, translation):
        with open(self.out_dir + '/' + name, "w") as f:
            f.write(translation)

    def dump_data(self, name, data, ext='data'):
        if not isinstance(data, list):
            data = [data]

        basedir = self.out_dir + '/data'
        util.mkdirp(basedir)
        with open(basedir + '/' + name + '.' + ext, "w") as f:
            for l in data:
                f.write(str(l) + '\n')

    def get_value_idx(self, value):
        """ Returns the appropriate integer index for the given value."""
        if isinstance(value, int):  # Variables of integer type are represented by the integer itself.
            return value
        else:
            # bool variables also need to be treated specially
            value = util.bool_string(value) if isinstance(value, bool) else value
            return self.index.objects[value]

    def get_function_instantiations(self):
        tpl = tplManager.get('function_instantiation')
        extensional = [
            tpl.substitute(name=name, accessor=elem.accessor)
            for name, elem in self.task.static_data.items() if isinstance(elem, DataElement)
        ]
        external = [tpl.substitute(
            name=symbol, accessor=symbol[1:]) for symbol in self.task.static_symbols if is_external(symbol)]

        return extensional + external

if __name__ == "__main__":
    main()
