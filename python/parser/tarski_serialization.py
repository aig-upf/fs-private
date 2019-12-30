import itertools
import os
from collections import defaultdict

from tarski.fstrips import AddEffect, DelEffect, FunctionalEffect, LiteralEffect, UniversalEffect
from tarski.syntax import Interval, Sort, util, Atom, Contradiction, Tautology, CompoundFormula, QuantifiedFormula, \
    Connective, CompoundTerm, Predicate, Constant, Variable, BuiltinPredicateSymbol
from tarski.syntax.sorts import inclusion_closure

from .. import utils
from . import static


def object_id(name, obj_idx):
    """ Return the numeric ID corresponding to the given object name, taking into account
    that it might be an string representing an integer or float literal. """
    try:
        return obj_idx[name]
    except KeyError:
        if utils.is_int(name):
            return int(name)
        elif utils.is_float(name):
            return float(name)
        raise


def tarski_sort_to_typeid(t: Sort):
    if not isinstance(t, Interval):
        return 'object_t'

    parenthood = set(inclusion_closure(t))  # This includes t itself
    if t.language.Integer in parenthood:
        return 'int_t'
    if t.language.Real in parenthood:
        return 'float_t'
    raise RuntimeError(f'Unknown Tarski sort "{t}"')


def tarski_sort_to_domain_type(t: Sort, type_id):
    if type_id == 'object_t':
        return 'set'
    if not isinstance(t, Interval):
        raise RuntimeError(f'Unknown domain type for Tarski sort "{t}"')
    return 'unbounded' if t in (t.language.Integer, t.language.Real) else 'interval'


def tarski_sort_to_domain(t: Sort, type_objects):
    if isinstance(t, Interval):
        setobj = []
        intobj = [] if t in (t.language.Integer, t.language.Real) else [t.lower_bound, t.upper_bound]

    elif isinstance(t, Sort):
        # Not sure why the backend requires the object ID as a string here, but so it seems to be
        setobj = [str(oid) for oid in type_objects[t.name]]
        intobj = []
    else:
        raise RuntimeError(f'Unknown domain for Tarski sort "{t}"')
    return setobj, intobj


def tarski_sort_to_typename(t: Sort):
    return {'Real': 'number', 'Integer': 'int'}.get(t.name, t.name)


def serialize_type_info(language, type_objects):
    type_idxs = dict()
    data = []
    # data = [
    #     {'id': 2, 'fstype': 'int', 'type_id': 'int_t', 'domain_type': 'unbounded', 'interval': [], 'set': []},
    #     {'id': 3, 'fstype': 'number', 'type_id': 'float_t', 'domain_type': 'unbounded', 'interval': [], 'set': []}
    # ]

    # Each typename is 1-indexed, since index 0 is reserved for the bool type
    # TODO We should check if this bool type is still necessary
    id_ = 1
    for sort in language.sorts:
        if (isinstance(sort, Interval) and sort.name == 'Natural') or\
                (isinstance(sort, Interval) and sort.name == 'number'):
            continue  # Natural and number types are not necessary for the backend

        type_idxs[sort] = id_
        typeid = tarski_sort_to_typeid(sort)
        domain_type = tarski_sort_to_domain_type(sort, typeid)
        setobj, intobj = tarski_sort_to_domain(sort, type_objects)
        fstype = tarski_sort_to_typename(sort)
        data.append(dict(id=id_, fstype=fstype, type_id=typeid, domain_type=domain_type, interval=intobj, set=setobj))
        id_ += 1

    return type_idxs, data


def serialize_object_info(language):
    data, index = [], {}
    type_objects = defaultdict(list)
    # Indexes 0 and 1 are reserved for two special boolean objects true and false
    # TODO Check if we still need these or can get rid of them
    for oid, o in enumerate(language.constants(), start=2):
        data.append(dict(id=oid, name=o.symbol, type=tarski_sort_to_typename(o.sort)))
        index[o.symbol] = oid
        for s in inclusion_closure(o.sort):
            type_objects[s.name].append(oid)
    return index, data, type_objects


def serialize_symbol_info(language, obj_idx, variables, statics):
    # We do two passes over the set of state variables, first to collect to which each symbol belongs,
    # and second to compile the relevant data for the backend
    symbol_variables = defaultdict(list)  # state variables derived from each symbol
    for varid, v in variables.enumerate():
        symbol_variables[v.symbol.name].append(varid)

    symdata, smb_idx = [], {}
    for sym in util.get_symbols(language, include_builtin=False):
        if sym.name == "total-cost":
            continue
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

    vardata = []
    for varid, v in variables.enumerate():
        sym = v.symbol
        fstype = 'bool' if isinstance(sym, Predicate) else sym.codomain.name
        point = [object_id(c.symbol, obj_idx) for c in v.binding]
        vardata.append(dict(id=varid, name=str(v), fstype=fstype, symbol_id=smb_idx[sym.symbol], point=point))

    return symdata, vardata, smb_idx


def serialize_tarski_formula(exp, obj_idx, binding, negated=False):
    # Formulas
    if isinstance(exp, Tautology):
        return dict(type='tautology')

    elif isinstance(exp, Contradiction):
        return dict(type='contradiction')

    elif isinstance(exp, Atom):
        children = [serialize_tarski_term(sub, obj_idx, binding) for sub in exp.subterms]
        sym = exp.symbol.name
        if isinstance(sym, BuiltinPredicateSymbol):
            if negated:  # We just negate by complementing the arithmetic symbol, e.g. from <= to >
                sym = sym.complement()
                negated = False
            sym = str(sym)
        return dict(type='atom', symbol=sym, children=children, negated=negated)

    elif isinstance(exp, CompoundFormula):
        if exp.connective == Connective.Not:
            return serialize_tarski_formula(exp.subformulas[0], obj_idx, binding, not negated)
        elif exp.connective in (Connective.And, Connective.Or):
            children = [serialize_tarski_formula(sub, obj_idx, binding, negated) for sub in exp.subformulas]
            t = 'and' if exp.connective == Connective.And else 'or'
            return dict(type=t, symbol=t, children=children, negated=False)

    elif isinstance(exp, QuantifiedFormula):
        pass

    raise RuntimeError(f'Unknown Tarski expression type {type(exp)} for expression {exp}')


def serialize_tarski_term(exp, obj_idx, binding):
    if isinstance(exp, Constant):
        return dict(type='constant', symbol=exp.symbol, type_id=tarski_sort_to_typeid(exp.sort),
                    fstype=tarski_sort_to_typename(exp.sort),
                    value=object_id(exp.symbol, obj_idx))

    elif isinstance(exp, CompoundTerm):
        children = [serialize_tarski_term(sub, obj_idx, binding) for sub in exp.subterms]
        # The str(·) cares for the case where we have a builtin symbols such as "+"
        return dict(type='functional', symbol=str(exp.symbol.name), children=children)

    elif isinstance(exp, Variable):
        return dict(type='variable', symbol=exp.symbol, fstype=tarski_sort_to_typename(exp.sort),
                    position=binding[exp.symbol])

    raise RuntimeError(f'Unknown Tarski expression type {type(exp)} for expression {exp}')


def serialize_tarski_model(init_model, variables, obj_idx, fluents, statics):
    init_sv_values = []
    init_data = dict()

    extensions = init_model.list_all_extensions()

    # First process the data in the model and create the old "FS" data structures
    # that will take care of the serialization.
    relevant = [s for s in itertools.chain(statics, fluents) if s.name != 'total-cost']
    for symbol in relevant:
        sname = symbol.name
        if isinstance(symbol, Predicate):
            d = static.instantiate_predicate(sname, symbol.arity)
            _ = [d.add(tuple(x.symbol for x in point), None) for point in extensions[symbol.signature]]
        else:
            d = static.instantiate_function(sname, symbol.arity)
            _ = [d.add(tuple(x.symbol for x in point[:-1]), point[-1].symbol) for point in extensions[symbol.signature]]

        if d.needs_serialization():
            init_data[symbol] = d.serialize_data(obj_idx)

    # Now compute the initial assignment of values to state variables
    for sv in variables:
        if isinstance(sv.symbol, Predicate):
            value = int(init_model.holds(sv.symbol, sv.binding))
        else:  # A function
            value = init_model.value(sv.symbol, sv.binding)
            value = object_id(value.name, obj_idx)
        varidx = variables.get_index(sv)
        init_sv_values.append([varidx, value])

    # For some reason it seems the backend expects the atoms to be sorted
    svars = dict(variables=len(variables), atoms=sorted(init_sv_values))
    return svars, init_data


def is_cost_effect(effect):
    """ Whether the given effect is a special total-cost effect. """
    return isinstance(effect, FunctionalEffect) and isinstance(effect.lhs, CompoundTerm) \
        and effect.lhs.symbol.name == "total-cost"


def serialize_tarski_effect(effect, obj_idx, binding):
    cond = serialize_tarski_formula(effect.condition, obj_idx, binding)

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
        return dict(type=t, condition=cond, lhs=lhs, rhs=rhs)

    elif isinstance(effect, LiteralEffect):
        pass
    elif isinstance(effect, FunctionalEffect):
        if is_cost_effect(effect):  # At the moment we simply ignore cost effects
            return None
        lhs = serialize_tarski_term(effect.lhs, obj_idx, binding)
        rhs = serialize_tarski_term(effect.rhs, obj_idx, binding)
        return dict(type='functional', condition=cond, lhs=lhs, rhs=rhs)
    elif isinstance(effect, UniversalEffect):
        pass
    raise RuntimeError(f'Unexpected type "{type(effect)}" for expression "{effect}"')


def serialize_tarski_action_schema(action, type_idx, obj_idx):
    signature = [type_idx[p.sort] for i, p in enumerate(action.parameters)]
    paramnames = [p.symbol for i, p in enumerate(action.parameters)]
    binding = {p.symbol: i for i, p in enumerate(action.parameters)}
    # TODO This binding unit information is not complete, and will not work for problems with conditional effects,
    #      quantified vars, etc. Check classes BindingUnit and FSActionSchema for the previous working implementation
    unit = [[i, p.symbol, tarski_sort_to_typename(p.sort)] for i, p in enumerate(action.parameters)]
    effects = []
    for eff in action.effects:
        serialized = serialize_tarski_effect(eff, obj_idx, binding)
        if serialized is not None:
            effects.append(serialized)
    return dict(name=action.name, signature=signature, type='control',
                parameters=paramnames,
                conditions=serialize_tarski_formula(action.precondition, obj_idx, binding, False),
                effects=effects,
                unit=unit)


def generate_tarski_problem(problem, fluents, statics, variables):
    """ Collect all the data relevant to the given tarski problem, and return a serialization-friendly
    dict-based representation """
    data = {'problem': {'domain': problem.domain_name, 'instance': problem.name}}

    obj_idx, data['objects'], type_objects = serialize_object_info(problem.language)

    data['symbols'], data['variables'], smb_idx = serialize_symbol_info(problem.language, obj_idx, variables, statics)

    type_idx, data['types'] = serialize_type_info(problem.language, type_objects)

    data['init'], init_atoms = serialize_tarski_model(problem.init, variables, obj_idx, fluents, statics)

    # No binding unit for the goal, which must be a sentence
    data['goal'] = dict(conditions=serialize_tarski_formula(problem.goal, obj_idx, {}, negated=False), unit=[])

    data['action_schemata'] = [serialize_tarski_action_schema(a, type_idx, obj_idx) for a in problem.actions.values()]

    # Some other data that we currently don't support or haven't ported from the old codebase yet
    data['transitions'] = []  # TODO self.dump_transition_data()
    data['axioms'] = []  # TODO [axiom.dump() for axiom in self.index.axioms]
    data['process_schemata'] = []
    data['event_schemata'] = []
    data['state_constraints'] = []
    data['metric'] = dict()

    return data, init_atoms, obj_idx


class Serializer:
    def __init__(self, basedir):
        self.basedir = basedir

    def compute_filenames(self, name, extension):
        return self.basedir, os.path.join(self.basedir, name + '.' + extension)

    def dump(self, name, data, extension):
        if extension == 'json' and not isinstance(data, str):
            import json
            data = json.dumps(data)
        data = data if isinstance(data, list) else [data]
        basedir, filename = self.compute_filenames(name, extension)
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


def print_groundings(schemas, groundings, obj_idx, serializer: Serializer):
    groundings_filename = "groundings"

    # If no groundings computed, remove the file from possible previous runs
    if groundings is None:
        _, filename = serializer.compute_filenames(groundings_filename, 'data')
        utils.silentremove(filename)
        return

    data = []

    # Order matters! The groundings of each action schema are provided in consecutive blocks, one grounding per line
    id_ = 0
    for action in schemas:
        action_name = action['name']
        schema_groundings = groundings[action_name]
        if not schema_groundings:
            continue
        data.append("# {} # {}".format(id_, action_name))  # A comment line
        id_ += 1

        action_groundings = []  # A list with the (integer index of) each grounding
        for grounding in groundings[action_name]:
            action_groundings.append(tuple(object_id(obj_name, obj_idx) for obj_name in grounding))

        for grounding in sorted(action_groundings):  # IMPORTANT to output the groundings in lexicographical order
            data.append(','.join(map(str, grounding)))

    serializer.dump(groundings_filename, data, extension='data')


def serialize_representation(data, init_atoms, serializer: Serializer, debug):
    serializer.dump('problem', data, extension='json')

    # For historical reasons, we used to serialize the list of static atoms separately and in a custom format
    # (namely, in combined task-and-motion planning problems, the amount of static data was so huge, that it paid
    # off to deal with it separately.
    # This is nowadays a terrible PITA though, but hopefully we'll get rid of it soon once we transition
    # to a Python-module based architecture
    for symbol, symbol_data in init_atoms.items():
        if isinstance(symbol, Predicate) and symbol.arity == 0:
            # For nullary atoms p, the hacky way the backend has to distinguish between the case where p holds and that
            # where it doesn't is to either have an empty serialized file or to have no file at all
            continue
        serializer.dump(symbol.name, symbol_data, extension='data')

    if debug:
        print_debug_data(data, Serializer(os.path.join(serializer.basedir, 'debug')))

    # TODO Reactivate this again if necessary
    # if self.requires_compilation():
    #     self.generate_components_code()
