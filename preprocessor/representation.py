"""
    A particular problem representation combining json-encoded data plus automatically-generated C++ code.
"""
import json
import operator

import util
from static import DataElement
from templates import tplManager
from util import is_external


class ProblemRepresentation(object):

    def __init__(self, index, translation_dir):
        self.index = index
        self.translation_dir = translation_dir

    def generate(self):

        data = {'variables': self.dump_variable_data(),
                'objects': self.dump_object_data(),
                'types': self.dump_type_data(),
                'action_schemata': self.index.action_schemas,
                'state_constraints': self.index.state_constraints,
                'init': self.dump_init_data(),
                'goal': self.index.goal,
                'functions': self.dump_function_data(),
                'problem': {'domain': self.index.domain_name, 'instance': self.index.instance_name}
                }

        self.dump_data('problem', json.dumps(data), ext='json')

        self.generate_components_code()

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
        processed = [getattr(elem, method)(self.index.objects.data) for elem in self.index.initial_static_data.values()
                     if isinstance(elem, DataElement)]
        return '\n\t'.join(processed)

    def get_external_data_initializer_list(self):
        elems = []
        for elem in self.index.initial_static_data.values():
            if isinstance(elem, DataElement):
                elems.append(elem.initializer_list())
            else:
                raise RuntimeError()
        if not elems:
            return ''
        return ': {}'.format(','.join(elems))

    def serialize_external_data(self):
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

    def dump_function_data(self):
        res = []
        for name, symbol in self.index.symbols.items():
            i = self.index.symbol_index[name]

            # Collect the list of variables that arise from this particular symbol
            f_variables = [(i, str(v)) for i, v in enumerate(self.index.state_variables) if v.symbol == name]

            static = name in self.index.static_symbols

            # Store the function info as a tuple:
            # <function_id, function_name, <function_domain>, function_codomain, state_variables>
            res.append([i, name, symbol.arguments, symbol.codomain, f_variables, static])
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

    def dump_data(self, name, data, ext='data'):
        if not isinstance(data, list):
            data = [data]

        basedir = self.translation_dir + '/data'
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

    def get_function_instantiations(self):
        tpl = tplManager.get('function_instantiation')
        extensional = [
            tpl.substitute(name=name, accessor=elem.accessor)
            for name, elem in self.index.initial_static_data.items() if isinstance(elem, DataElement)
            ]

        external = [tpl.substitute(
            name=symbol, accessor=symbol[1:]) for symbol in self.index.static_symbols if is_external(symbol)]

        return extensional + external
