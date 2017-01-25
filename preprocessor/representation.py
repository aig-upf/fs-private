"""
    A particular problem representation combining json-encoded data plus automatically-generated C++ code.
"""
import json
import operator

import base
import util
from static import DataElement
from templates import tplManager
from util import is_external


class ProblemRepresentation(object):

    def __init__(self, index, translation_dir, edebug):
        self.index = index
        self.translation_dir = translation_dir
        self.edebug = edebug

    def generate(self):

        data = {'variables': self.dump_variable_data(),
                'objects': self.dump_object_data(),
                'types': self.dump_type_data(),
                'action_schemata': self.index.action_schemas,
                'state_constraints': self.index.state_constraints,
                'init': self.dump_init_data(),
                'goal': self.index.goal,
                'symbols': self.dump_symbol_data(),
                'problem': {'domain': self.index.domain_name, 'instance': self.index.instance_name}
                }

        self.dump_data('problem', json.dumps(data), ext='json')

        # Optionally, we'll want to print out the precomputed action groundings
        self.print_groundings_if_available(self.index.action_schemas, self.index.groundings, self.index.objects)
        self.print_debug_data(data)
        self.serialize_static_extensions()

        if self.requires_compilation():
            self.generate_components_code()

    def generate_components_code(self):
        # components.hxx:
        self.save_translation('components.hxx', tplManager.get('components.hxx').substitute(
            method_factories=self.get_method_factories(),
        ))

        # components.cxx:
        self.save_translation('components.cxx', tplManager.get('components.cxx').substitute())

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
        return dict(variables=len(self.index.state_variables), atoms=indexed_atoms)

    def dump_variable_data(self):
        res = []
        for i, var in enumerate(self.index.state_variables):
            data = self.dump_state_variable(var)
            res.append({'id': i, 'name': str(var), 'type': self.index.symbol_types[var.symbol], 'data': data})
        return res

    def dump_state_variable(self, var):
        head = self.index.symbol_index[var.symbol]
        constants = [arg if util.is_int(arg) else self.index.objects.get_index(arg) for arg in var.args]
        return [head, constants]

    def dump_object_data(self):
        return [{'id': i, 'name': obj} for i, obj in enumerate(self.index.objects.dump())]

    def dump_symbol_data(self):
        res = []
        for name, symbol in self.index.symbols.items():
            i = self.index.symbol_index[name]

            type_ = "predicate" if isinstance(symbol, base.Predicate) else "function"

            # Collect the list of variables that arise from this particular symbol
            f_variables = [(i, str(v)) for i, v in enumerate(self.index.state_variables) if v.symbol == name]

            static = name in self.index.static_symbols

            # Store the symbol info as a tuple:
            # <symbol_id, symbol_name, symbol_type, <function_domain>, function_codomain, state_variables, static?>
            res.append([i, name, type_, symbol.arguments, symbol.codomain, f_variables, static])
        return res

    def dump_type_data(self):
        """ Dumps a map of types to corresponding objects"""
        data = []
        _sorted = sorted(self.index.types.items(), key=operator.itemgetter(1))  # all types, sorted by type ID
        for t, i in _sorted:
            objects = self.index.type_map[t]
            if objects and isinstance(objects[0], int):  # We have a bounded int variable
                data.append([i, t, 'int', [objects[0], objects[-1]]])
            else:
                object_idxs = [str(self.index.objects.get_index(o)) for o in objects]
                data.append([i, t, object_idxs])

        return data

    def save_translation(self, name, translation):
        with open(self.translation_dir + '/' + name, "w") as f:
            f.write(translation)

    def dump_data(self, name, data, ext='data', subdir=None):
        if not isinstance(data, list):
            data = [data]

        basedir = self.translation_dir + '/data'

        if subdir:
            basedir += '/' + subdir

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
            return self.index.objects.get_index(value)

    def requires_compilation(self):
        # The problem requires compilation iff there are external symbols involved.
        return len([s for s in self.index.static_symbols if is_external(s)])

    def get_function_instantiations(self):
        return [tplManager.get('function_instantiation').substitute(name=symbol, accessor=symbol[1:])
                for symbol in self.index.static_symbols if is_external(symbol)]

    def print_debug_data(self, data):
        if not self.edebug:
            return

        self.dump_data('debug.problem', json.dumps(data, indent=2), ext='json', subdir='debug')
        # self.dump_data('debug.problem', pprint.pformat(data, width=30, indent=2), ext='txt', subdir='debug')

        # For ease of inspection we dump each component into a separate JSON file
        for k, elem in data.items():
            self.dump_data("{}".format(k), json.dumps(elem, indent=2), ext='json', subdir='debug')

        # And further separate each action into a different file:
        for action in data['action_schemata']:
            self.dump_data("action.{}".format(action['name']), json.dumps(action, indent=2), ext='json', subdir='debug')

    def print_groundings_if_available(self, schemas, all_groundings, object_idx):

        if all_groundings is None:  # No groundings available
            return

        data = []

        # Order matters! The groundings of each action schema are provided in consecutive blocks, one grounding per line
        for i, action in enumerate(schemas, 0):
            action_name = action['name']
            data.append("# {} # {}".format(i, action_name))  # A comment line

            for grounding in all_groundings[action_name]:
                object_ids = (str(object_idx.get_index(obj_name)) for obj_name in grounding)
                data.append(','.join(object_ids))

        self.dump_data("groundings", data)


