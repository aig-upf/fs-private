from collections import defaultdict

from tarski.fstrips import AddEffect, DelEffect, FunctionalEffect, LiteralEffect, UniversalEffect
from tarski.grounding.common import StateVariableLite
from tarski.syntax import Interval, Sort, util, Atom, Contradiction, Tautology, CompoundFormula, QuantifiedFormula, \
    Connective, CompoundTerm, Predicate, Constant, Variable
from tarski.syntax.sorts import ancestors


def serialize_type_info(language, type_objects):
    type_idxs = dict()
    data = []

    # Declare language sorts
    for sort in language.sorts:
        if isinstance(sort, Interval):
            if sort.name in ('Real', 'Integer', 'Natural'):
                continue  # Numeric types are primitive types, not FSTRIPS types
            # TODO
            raise RuntimeError("TODO: Don't ignore interval types.")

        elif isinstance(sort, Sort):
            typeid = len(type_idxs)
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
    for o in language.constants():
        oid = len(index)
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
        symbol_variables[v.symbol.symbol].append(varid)

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

        static = sym in statics

        # This was used only for a very niche use case we don't exercise anymore, symbols that started with "@@"
        # and were defined as semantic attachments with access to the full state
        unbounded = False

        # Store the symbol info as a tuple:
        # <ID, name, type, <function_domain>, function_codomain, state_variables, static?, unbounded_arity?>
        symdata.append([sid, sym.symbol, type_, domain, codomain, symbol_variables[sym.symbol], static, unbounded])

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
        return dict(type='atom', symbol=exp.symbol.symbol, children=children, negated=negated)

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


def serialize_tarski_model(model, var_idx, obj_idx):
    init_atoms = []
    for atom in model.as_atoms():
        if isinstance(atom, tuple):  # We have a functional state variable
            varidx = var_idx[StateVariableLite(atom[0].symbol, atom[0].subterms)]
            value = obj_idx[atom[1]]
        else:
            varidx = var_idx[StateVariableLite(atom.symbol, atom.subterms)]
            value = 1
        init_atoms.append([varidx, value])

    # For some reason it seems the backend expects the atoms to be sorted
    sorted_atoms = sorted(init_atoms)
    return dict(variables=len(var_idx), atoms=sorted_atoms)


def serialize_tarski_effect(effect, obj_idx, binding):
    if isinstance(effect, (AddEffect, DelEffect)):
        if isinstance(effect, DelEffect):
            t = 'del'
            rhs = {'type': 'constant', 'symbol': 'false', 'value': 0, 'type_id': 'bool_t', 'fstype': 'bool'}
        else:
            t = 'add'
            rhs = {'type': 'constant', 'symbol': 'true', 'value': 1, 'type_id': 'bool_t', 'fstype': 'bool'}

        # TODO This is a HACK by which we replace an atom of the form visited(c) by a fake functional effect
        # TODO visited(c) := true
        children = [serialize_tarski_term(sub, obj_idx, binding) for sub in effect.atom.subterms]
        lhs = dict(type='functional', symbol=effect.atom.symbol, children=children)

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
    signature = [type_idx[x.sort] for x in action.parameters]
    paramnames = [x.symbol for x in action.parameters]
    binding = {name: pos for pos, name in enumerate(paramnames)}
    return dict(name=action.name, signature=signature, type='control',
                parameters=paramnames,
                conditions=serialize_tarski_formula(action.precondition, obj_idx, binding, False),
                effects=[serialize_tarski_effect(eff, obj_idx, binding) for eff in action.effects],
                unit=[])  # TODO Binding Unit is missing here


def serialize_tarski_problem(problem, statics, variables, ground_actions, directory, debug):
    data = {'problem': {'domain': problem.domain_name, 'instance': problem.name}}

    obj_idx, data['objects'], type_objects = serialize_object_info(problem.language)
    data['symbols'], data['variables'], smb_idx, var_idx = serialize_symbol_info(problem.language, obj_idx, variables, statics)
    type_idx, data['types'] = serialize_type_info(problem.language, type_objects)

    data['transitions'] = None  # TODO self.dump_transition_data()
    data['axioms'] = None  # TODO [axiom.dump() for axiom in self.index.axioms]

    data['init'] = serialize_tarski_model(problem.init, var_idx, obj_idx)

    # No binding unit for the goal, which must be a sentence
    data['goal'] = dict(conditions=serialize_tarski_formula(problem.goal, obj_idx, {}, negated=False), unit=[])

    data['action_schemata'] = [serialize_tarski_action_schema(a, type_idx, obj_idx) for a in problem.actions.values()]

    # TODO: Now write this data structure to the working directory
    return data
