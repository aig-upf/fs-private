import os
from collections import defaultdict

from tarski.fstrips import AddEffect, DelEffect, FunctionalEffect, LiteralEffect, UniversalEffect
from tarski.grounding.common import StateVariableLite
from tarski.syntax import Interval, Sort, util, Atom, Contradiction, Tautology, CompoundFormula, QuantifiedFormula, \
    Connective, CompoundTerm, Predicate, Constant, Variable
from tarski.syntax.sorts import ancestors

from .. import utils
from . import static


def serialize_type_info(language, type_objects):
    type_idxs = dict()
    data = []
    # data = [
    #     {'id': 2, 'fstype': 'int', 'type_id': 'int_t', 'domain_type': 'unbounded', 'interval': [], 'set': []},
    #     {'id': 3, 'fstype': 'number', 'type_id': 'float_t', 'domain_type': 'unbounded', 'interval': [], 'set': []}
    # ]

    # Each typename is 1-indexed, since index 0 is reserved for the bool type
    # TODO We should check if this bool type is still necessary
    for typeid, sort in enumerate(language.sorts, start=1):
        if isinstance(sort, Interval):
            if sort.name in ('Real', 'Integer', 'Natural'):
                continue  # Numeric types are primitive types, not FSTRIPS types
            raise RuntimeError("TODO: Don't ignore interval types.")  # TODO

        elif isinstance(sort, Sort):
            type_idxs[sort] = typeid
            # Not sure why the backend requires the object ID as a string here, but so it seems to be
            sort_objects = [str(oid) for oid in type_objects[sort.name]]
            data.append(dict(id=typeid, fstype=sort.name, type_id='object_t', domain_type='set', interval=[], set=sort_objects))
        else:
            raise RuntimeError("Unknown sort type: {}".format(sort))

    return type_idxs, data


def serialize_object_info(language):
    data, index = [], {}
    type_objects = defaultdict(list)
    # Indexes 0 and 1 are reserved for two special boolean objects true and false
    # TODO Check if we still need these or can get rid of them
    for oid, o in enumerate(language.constants(), start=2):
        data.append(dict(id=oid, name=o.symbol, type=o.sort.name))
        index[o.symbol] = oid
        type_objects[o.sort.name].append(oid)
        for s in ancestors(o.sort):
            type_objects[s.name].append(o.symbol)
    return index, data, type_objects


def serialize_symbol_info(language, obj_idx, variables, statics):

    # We do two passes over the set of state variables, one to collect to  which symbol belongs each,
    # and later another one to compile the relevant data for the backend
    symbol_variables = defaultdict(list)
    for varid, v in variables.enumerate():
        symbol_variables[v.symbol.name].append(varid)

    symdata, smb_idx = [], {}
    for sym in util.get_symbols(language, include_builtin=False):
        sid = len(smb_idx)
        smb_idx[sym.symbol] = sid
        if isinstance(sym, Predicate):
            type_ = "predicate"
            domain = [t.name for t in sym.domain]
            codomain = 'bool'
        else:
            type_ = "function"
            domain = [t.name for t in sym.domain]
            codomain = sym.codomain.name

        is_static = sym in statics

        # This was used only for a very niche use case we don't exercise anymore, symbols that started with "@@"
        # and were defined as semantic attachments with access to the full state
        unbounded = False

        # Store the symbol info as a tuple:
        # <ID, name, type, <function_domain>, function_codomain, state_variables, static?, unbounded_arity?>
        symdata.append([sid, sym.symbol, type_, domain, codomain, symbol_variables[sym.symbol], is_static, unbounded])

    vardata, var_idx = [], {}
    for varid, v in variables.enumerate():
        sym = v.symbol
        fstype = 'bool' if isinstance(sym, Predicate) else sym.codomain.name
        point = [obj_idx[c.symbol] for c in v.binding]
        vardata.append(dict(id=varid, name=str(v), fstype=fstype, symbol_id=smb_idx[sym.symbol], point=point))
        var_idx[v] = varid

    return symdata, vardata, smb_idx, var_idx


def serialize_tarski_formula(exp, obj_idx, binding, negated=False):
    # Formulas
    if isinstance(exp, Tautology):
        return dict(type='tautology')

    elif isinstance(exp, Contradiction):
        return dict(type='contradiction')

    elif isinstance(exp, Atom):
        children = [serialize_tarski_term(sub, obj_idx, binding) for sub in exp.subterms]
        return dict(type='atom', symbol=exp.symbol.name, children=children, negated=negated)

    elif isinstance(exp, CompoundFormula):
        if exp.connective == Connective.Not:
            return serialize_tarski_formula(exp, obj_idx, binding, not negated)
        elif exp.connective in (Connective.And, Connective.Or):
            children = [serialize_tarski_formula(sub, obj_idx, binding, negated) for sub in exp.subformulas]
            t = 'and' if exp.connective == Connective.And else 'or'
            return dict(type=t, symbol=t, children=children, negated=False)

    elif isinstance(exp, QuantifiedFormula):
        pass

    raise RuntimeError(f'Unknown Tarski expression type {type(exp)} for expression {exp}')


def serialize_tarski_term(exp, obj_idx, binding):
    if isinstance(exp, Constant):
        return dict(type='constant', symbol=exp.symbol, type_id="object_t", fstype=exp.sort.name, 
                    value=obj_idx[exp.symbol])

    elif isinstance(exp, CompoundTerm):
        children = [serialize_tarski_term(sub, obj_idx, binding) for sub in exp.subterms]
        return dict(type='functional', symbol=exp.symbol, children=children)

    elif isinstance(exp, Variable):
        return dict(type='variable', symbol=exp.symbol, fstype=exp.sort.name, position=binding[exp.symbol])

    raise RuntimeError(f'Unknown Tarski expression type {type(exp)} for expression {exp}')


def serialize_tarski_model(model, var_idx, obj_idx, fluents, statics):
    init_atoms = []
    static_data = dict()

    extensions = model.list_all_extensions()

    # First process the data that is static in the model, and create the old "FS" data structures
    # that will take care of the serialization.
    for symbol in statics:
        sname = symbol.name
        if isinstance(symbol, Predicate):
            d = static.instantiate_predicate(sname, symbol.arity)
            _ = [d.add(tuple(x.symbol for x in point), None) for point in extensions[symbol.signature]]
        else:
            d = static.instantiate_function(sname, symbol.arity)
            _ = [d.add(tuple(x.symbol for x in point[:-1]), point[-1].symbol) for point in extensions[symbol.signature]]

        static_data[symbol] = d.serialize_data(obj_idx)

    # And now process the fluent data in the model.
    for symbol in fluents:
        for point in extensions[symbol.signature]:
            if isinstance(symbol, Predicate):
                varidx = var_idx[StateVariableLite(symbol, point)]
                value = 1
            else:
                varidx = var_idx[StateVariableLite(symbol, point[:-1])]
                value = obj_idx[point[-1]]
            init_atoms.append([varidx, value])

    # For some reason it seems the backend expects the atoms to be sorted
    init_data = dict(variables=len(var_idx), atoms=sorted(init_atoms))
    return init_data, static_data


def serialize_tarski_effect(effect, obj_idx, binding):
    if isinstance(effect, (AddEffect, DelEffect)):
        if isinstance(effect, DelEffect):
            t = 'del'
            rhs = {'type': 'constant', 'symbol': 'false', 'value': 0, 'type_id': 'bool_t', 'fstype': 'bool'}
        else:
            t = 'add'
            rhs = {'type': 'constant', 'symbol': 'true', 'value': 1, 'type_id': 'bool_t', 'fstype': 'bool'}

        # TODO This is a HACK by which we replace an atom of the form visited(c) by a fake functional effect
        #      visited(c) := true
        children = [serialize_tarski_term(sub, obj_idx, binding) for sub in effect.atom.subterms]
        lhs = dict(type='functional', symbol=effect.atom.symbol.name, children=children)

        return dict(type=t, condition=serialize_tarski_formula(effect.condition, obj_idx, binding), lhs=lhs, rhs=rhs)

    elif isinstance(effect, LiteralEffect):
        pass
    elif isinstance(effect, FunctionalEffect):
        pass
    elif isinstance(effect, UniversalEffect):
        pass
    else:
        raise RuntimeError(f'Unexpected type "{type(effect)}" for expression "{effect}"')


def serialize_tarski_action_schema(action, type_idx, obj_idx):
    signature = [type_idx[p.sort] for i, p in enumerate(action.parameters)]
    paramnames = [p.symbol for i, p in enumerate(action.parameters)]
    binding = {p.symbol: i for i, p in enumerate(action.parameters)}
    # TODO This binding unit information is not complete, and will not work for problems with conditional effects,
    #      quantified vars, etc. Check classes BindingUnit and FSActionSchema for the previous working implementation
    unit = [[i, p.symbol, p.sort.name] for i, p in enumerate(action.parameters)]
    return dict(name=action.name, signature=signature, type='control',
                parameters=paramnames,
                conditions=serialize_tarski_formula(action.precondition, obj_idx, binding, False),
                effects=[serialize_tarski_effect(eff, obj_idx, binding) for eff in action.effects],
                unit=unit)


def generate_tarski_problem(problem, fluents, statics, variables, ground_actions):
    """ Collect all the data relevant to the given tarski problem, and return a serialization-friendly
    dict-based representation """
    data = {'problem': {'domain': problem.domain_name, 'instance': problem.name}}

    obj_idx, data['objects'], type_objects = serialize_object_info(problem.language)
    data['symbols'], data['variables'], smb_idx, var_idx = serialize_symbol_info(problem.language, obj_idx, variables, statics)
    type_idx, data['types'] = serialize_type_info(problem.language, type_objects)

    data['transitions'] = []  # TODO self.dump_transition_data()
    data['axioms'] = []  # TODO [axiom.dump() for axiom in self.index.axioms]
    data['process_schemata'] = []
    data['event_schemata'] = []
    data['state_constraints'] = []
    data['metric'] = dict()

    data['init'], static_atoms = serialize_tarski_model(problem.init, var_idx, obj_idx, fluents, statics)

    # No binding unit for the goal, which must be a sentence
    data['goal'] = dict(conditions=serialize_tarski_formula(problem.goal, obj_idx, {}, negated=False), unit=[])

    data['action_schemata'] = [serialize_tarski_action_schema(a, type_idx, obj_idx) for a in problem.actions.values()]

    return data, static_atoms


class Serializer:
    def __init__(self, basedir):
        self.basedir = basedir

    def _compute_filenames(self, name, ext):
        return self.basedir, os.path.join(self.basedir, name + '.' + ext)

    def dump(self, name, data, extension):
        if extension == 'json' and not isinstance(data, str):
            import json
            data = json.dumps(data)
        data = data if isinstance(data, list) else [data]
        basedir, filename = self._compute_filenames(name, extension)
        utils.mkdirp(basedir)
        with open(filename, "w") as f:
            for l in data:
                f.write(str(l) + '\n')


def print_debug_data(data, serializer: Serializer):
    import json
    serializer.dump('debug.problem', json.dumps(data, indent=2), extension='json')

    # For ease of inspection we dump each component into a separate JSON file
    for k, elem in data.items():
        serializer.dump("{}".format(k), json.dumps(elem, indent=2), extension='json')

    # And further separate each action into a different file:
    for action in data['action_schemata']:
        serializer.dump("action.{}".format(action['name']), json.dumps(action, indent=2), extension='json')

    # Separate each axiom into a different file:
    # for axiom in data['axioms']:
    #     serializer.dump("axiom.{}".format(axiom['name']), json.dumps(axiom, indent=2), extension='json')

    # Print all (ordered) action schema names in a separate file
    names = [action['name'] for action in data['action_schemata']]
    serializer.dump("schemas", names, extension='txt')

    # Types
    lines = list()
    for t in data['types']:
        lines.append('{}'.format(t))
        # objects = t[2]
        # if objects == 'int':
        #    objects = "int[{}..{}]".format(t[3][0], t[3][1])
        # lines.append("{}: {}. Objects: {}".format(t[0], t[1], objects))
    serializer.dump("types", lines, extension='txt')

    # Variables
    lines = list()
    for elem in data['variables']:
        lines.append('{}: "{}" ({})'.format(elem['id'], elem['name'], elem['fstype']))
    serializer.dump("variables", lines, extension='txt')

    # Symbols
    lines = list()
    for elem in data['symbols']:
        signature = ', '.join(elem[3])
        lines.append('{}: "{}" ({}). Signature: ({})'.format(elem[0], elem[1], elem[2], signature))
    serializer.dump("symbols", lines, extension='txt')

    # Objects
    lines = list()
    for elem in data['objects']:
        lines.append('{}: "{}"'.format(elem['id'], elem['name']))
    serializer.dump("objects", lines, extension='txt')

    # Init
    lines = list()
    for elem in data['init']['atoms']:
        lines.append('Variable {} has value "{}"'.format(elem[0], elem[1]))
    serializer.dump("init", lines, extension='txt')

    # Transitions
    lines = list()
    for var_transitions in data['transitions']:
        lines.append('{}: "{}"'.format(var_transitions[0], var_transitions[1]))
    serializer.dump("transitions", lines, extension='txt')


def serialize_representation(data, static_atoms, workdir, debug):
    serializer = Serializer(os.path.join(workdir, 'data'))
    serializer.dump('problem', data, extension='json')

    # For historical reasons, we used to serialize the list of static atoms separately and in a custom format
    # (namely, in combined task-and-motion planning problems, the amount of static data was so huge, that it paid
    # off to deal with it separately.
    # This is nowadays a terrible PITA though, but hopefully we'll get rid of it soon once we transition
    # to a Python-module based architecture
    for symbol, static_data in static_atoms.items():
        if isinstance(symbol, Predicate) and symbol.arity == 0:
            # For nullary atoms p, the hacky way the backend has to distinguish between the case where p holds and that
            # where it doesn't is to either have an empty serialized file or to have no file at all
            continue
        serializer.dump(symbol.name, static_data, extension='data')

    # Optionally, we'll want to print out the precomputed action groundings
    # print_groundings_if_available(data['action_schemata'], self.index.groundings, self.index.objects)
    if debug:
        print_debug_data(data, Serializer(os.path.join(workdir, 'data', 'debug')))

    # TODO Reactivate this again if necessary
    # if self.requires_compilation():
    #     self.generate_components_code()
