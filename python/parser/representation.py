"""
    A particular problem representation combining json-encoded data plus automatically-generated C++ code.
"""
import json
import operator

from python import utils
from python.parser.parser import Parser
from . import fstrips
from . import util
from .static import DataElement
from .templates import tplManager
from .pddl.pddl_types import TypedObject


class ProblemRepresentation(object):
    def __init__(self, index, translation_dir, debug):
        self.index = index
        self.translation_dir = translation_dir
        self.debug = debug

    def generate(self):

        data = {'variables': self.dump_variable_data(),
                'objects': self.dump_object_data(),
                'types': self.dump_type_data(),
                'action_schemata': [action.dump() for action in self.index.action_schemas],
                'process_schemata': [process.dump() for process in self.index.process_schemas],
                'event_schemata': [event.dump() for event in self.index.event_schemas],
                'metric': self.index.metric.dump(),
                'state_constraints': [constraint.dump() for constraint in self.index.state_constraints],
                'goal': self.index.goal.dump(),
                'axioms': [axiom.dump() for axiom in self.index.axioms],
                'init': self.dump_init_data(),
                'symbols': self.dump_symbol_data(),

                'problem': {'domain': self.index.domain_name, 'instance': self.index.instance_name}
                }

        self.dump_data('problem', json.dumps(data), ext='json')

        # Optionally, we'll want to print out the precomputed action groundings
        self.print_groundings_if_available(data['action_schemata'], self.index.groundings, self.index.objects)
        self.print_debug_data(data)
        self.serialize_static_extensions()

        if self.requires_compilation():
            self.generate_components_code()

    def generate_components_code(self):
        # components.hxx:
        self.save_translation('components.hxx', tplManager.get('components.hxx').substitute(
            method_factories=self.get_method_factories(),
        ))

    def serialize_static_extensions(self):
        for elem in self.index.initial_static_data.values():
            assert isinstance(elem, DataElement)
            serialized = elem.serialize_data(self.index.objects.data)
            self.dump_data(elem.name, serialized)

    def get_method_factories(self):
        return tplManager.get('factories').substitute(
            functions=',\n\t\t\t'.join(self.get_function_instantiations()),
        )

    def dump_init_data(self):
        """ Saves the initial values of all state variables explicitly mentioned in the initialization """

        def indexer(atom):  # A small helper
            return [self.index.state_variables.get_index(atom.var), self.get_value_idx(atom.value)]

        indexed_atoms = [indexer(atom) for atom in self.index.initial_fluent_atoms]
        sorted_atoms = sorted(indexed_atoms)
        return dict(variables=len(self.index.state_variables), atoms=sorted_atoms)

    def dump_variable_data(self):
        all_variables = []
        for i, var in enumerate(self.index.state_variables):
            signature = [self.index.object_types[arg] for arg in var.args]
            point = [arg if utils.is_int(arg) else self.index.objects.get_index(arg) for arg in var.args]

            all_variables.append(dict(id=i, name=str(var),
                                      fstype=self.index.symbol_types[var.symbol],
                                      symbol_id=self.index.symbol_index[var.symbol],
                                      signature=signature,
                                      point=point))

        return all_variables

    def dump_object_data(self):
        types = self.index.object_types

        def dump_o(i, obj):
            if obj in ('true', 'false'):  # DON'T DUMP BOOLEAN OBJECTS
                return None
            return dict(id=i, name=obj, type=types[obj])

        all_objs = [dump_o(i, obj) for i, obj in enumerate(self.index.objects.dump())]
        return [x for x in all_objs if x is not None]

    def dump_symbol_data(self):
        res = []
        p = Parser(self.index)
        for name, symbol in self.index.symbols.items():
            i = self.index.symbol_index[name]

            type_ = "predicate" if isinstance(symbol, fstrips.Predicate) else "function"

            # Collect the list of variables that arise from this particular symbol
            f_variables = [(i, str(v)) for i, v in enumerate(self.index.state_variables) if v.symbol == name]

            static = name in self.index.static_symbols
            unbounded = util.has_unbounded_arity(name)

            # Store the symbol info as a tuple:
            # <ID, name, type, <function_domain>, function_codomain, state_variables, static?, unbounded_arity?>
            res.append([i, name, type_, symbol.arguments, symbol.codomain, f_variables, static, unbounded])
        return res

    def dump_type_data(self):
        """ Dumps a map of types to corresponding objects"""

        def resolve_type_id(type):
            if type == 'int': return 'int_t'
            if type == 'number': return 'float_t'
            if type == 'object': return 'object_t'
            if 'int' in self.index.supertypes[type]: return 'int_t'
            if 'number' in self.index.supertypes[type]: return 'float_t'
            if 'object' in self.index.supertypes[type]: return 'object_t'
            assert False

        def resolve_domain_type(type_id, objects):
            if len(objects) == 0: return 'unbounded'
            if isinstance(objects[0], int): return 'interval'
            return 'set'

        data = []
        _sorted = sorted(self.index.types.items(), key=operator.itemgetter(1))  # all types, sorted by type ID
        for t, i in _sorted:
            objects = self.index.type_map[t]
            type_def_components = [("id", i), ("fstype", t)]

            type_id = resolve_type_id(t)
            type_def_components += [("type_id", type_id)]

            dom_type = resolve_domain_type(type_id, objects)
            type_def_components += [("domain_type", dom_type)]

            if dom_type == 'interval':
                type_def_components += [("interval", [objects[0], objects[-1]]), ("set", [])]
            elif dom_type == 'set':
                object_idxs = [str(self.index.objects.get_index(o)) for o in objects]
                type_def_components += [("interval", []), ("set", object_idxs)]
            elif dom_type == 'unbounded':
                type_def_components += [("interval", []), ("set", [])]
            else:
                assert False

            data.append(dict(type_def_components))

        return data

    def save_translation(self, name, translation):
        with open(self.translation_dir + '/' + name, "w") as f:
            f.write(translation)

    def _compute_filenames(self, name, ext, subdir=None):
        basedir = self.translation_dir + '/data'

        if subdir:
            basedir += '/' + subdir

        return basedir, basedir + '/' + name + '.' + ext

    def dump_data(self, name, data, ext='data', subdir=None):
        if not isinstance(data, list):
            data = [data]

        basedir, filename = self._compute_filenames(name, ext, subdir)
        utils.mkdirp(basedir)
        with open(filename, "w") as f:
            for l in data:
                f.write(str(l) + '\n')

    def rm_data(self, name, ext='data', subdir=None):
        basedir, filename = self._compute_filenames(name, ext, subdir)
        utils.silentremove(filename)

    def get_value_idx(self, value):
        """ Returns the appropriate integer index for the given value."""
        # Variables of integer and float type are represented by the integer itself.
        if isinstance(value, int) or isinstance(value, float):
            return value

        elif isinstance(value, bool):
            return util.bool_string(value)

        elif isinstance(value, TypedObject):
            return self.index.objects.get_index(value.name)
        else:
            return self.index.objects.get_index(value)

    def requires_compilation(self):
        # The problem requires compilation iff there are external symbols involved.
        for s in self.index.all_symbols :
            if util.is_external(s) : return True
        for a in self.index.action_schemas :
            if util.is_external(a.action.name) : return True
        for a in self.index.event_schemas :
            if util.is_external(a.action.name) : return True
        return False

    def get_function_instantiations(self):
        return [tplManager.get('function_instantiation').substitute(name=symbol, accessor=symbol.replace('@', ''))
                for symbol in self.index.static_symbols if util.is_external(symbol)]

    def print_debug_data(self, data):
        if not self.debug:
            return

        self.dump_data('debug.problem', json.dumps(data, indent=2), ext='json', subdir='debug')
        # self.dump_data('debug.problem', pprint.pformat(data, width=30, indent=2), ext='txt', subdir='debug')

        # For ease of inspection we dump each component into a separate JSON file
        for k, elem in data.items():
            self.dump_data("{}".format(k), json.dumps(elem, indent=2), ext='json', subdir='debug')

        # And further separate each action into a different file:
        for action in data['action_schemata']:
            self.dump_data("action.{}".format(action['name']), json.dumps(action, indent=2), ext='json', subdir='debug')

        # Separate each axiom into a different file:
        for axiom in data['axioms']:
            self.dump_data("axiom.{}".format(axiom['name']), json.dumps(axiom, indent=2), ext='json', subdir='debug')

        # Print all (ordered) action schema names in a separate file
        names = [action['name'] for action in data['action_schemata']]
        self.dump_data("schemas", names, ext='txt', subdir='debug')

        # Print all (ordered) axioms in a separate file
        axiom_data = ["{}\n\n".format(axiom) for axiom in self.index.axioms]
        self.dump_data("axioms", axiom_data, ext='txt', subdir='debug')

        # Types
        lines = list()
        for t in data['types']:
            lines.append('{}'.format(t))
            # objects = t[2]
            # if objects == 'int':
            #    objects = "int[{}..{}]".format(t[3][0], t[3][1])
            # lines.append("{}: {}. Objects: {}".format(t[0], t[1], objects))
        self.dump_data("types", lines, ext='txt', subdir='debug')

        # Variables
        lines = list()
        for elem in data['variables']:
            lines.append('{}: "{}" ({})'.format(elem['id'], elem['name'], elem['fstype']))
        self.dump_data("variables", lines, ext='txt', subdir='debug')

        # Symbols
        lines = list()
        for elem in data['symbols']:
            signature = ', '.join(elem[3])
            lines.append('{}: "{}" ({}). Signature: ({})'.format(elem[0], elem[1], elem[2], signature))
        self.dump_data("symbols", lines, ext='txt', subdir='debug')

        # Objects
        lines = list()
        for elem in data['objects']:
            lines.append('{}: "{}"'.format(elem['id'], elem['name']))
        self.dump_data("objects", lines, ext='txt', subdir='debug')

        # Init
        lines = list()
        for elem in data['init']['atoms']:
            lines.append('Variable {} has value "{}"'.format(elem[0], elem[1]))
        self.dump_data("init", lines, ext='txt', subdir='debug')

    def print_groundings_if_available(self, schemas, all_groundings, object_idx):
        groundings_filename = "groundings"

        if all_groundings is None:  # No groundings available
            self.rm_data(groundings_filename)
            return

        data = []

        # Order matters! The groundings of each action schema are provided in consecutive blocks, one grounding per line
        for i, action in enumerate(schemas, 0):
            action_name = action['name']
            data.append("# {} # {}".format(i, action_name))  # A comment line

            action_groundings = []  # A list with the (integer index of) each grounding
            for grounding in all_groundings[action_name]:
                action_groundings.append(tuple(object_idx.get_index(obj_name) for obj_name in grounding))

            for grounding in sorted(action_groundings):  # IMPORTANT to output the groundings in lexicographical order
                data.append(','.join(map(str, grounding)))

        self.dump_data(groundings_filename, data)
