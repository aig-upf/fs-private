"""
    Translation of Tarski planning problems into the FS planner native format
"""
import logging

import tarski
import tarski.syntax as tsk
from tarski.fstrips import DelEffect, AddEffect
from tarski.syntax import util

from .. import extension as cext


class LanguageInfoWrapper:
    def __init__(self, linfo, type_idxs, obj_idxs, symbol_idxs):
        self.linfo = linfo
        self.type_idxs = type_idxs
        self.obj_idxs = obj_idxs
        self.symbol_idxs = symbol_idxs


def create_language_info(language):
    logging.info("Translating language information to FS internal format...")
    info = cext.LanguageInfo()

    type_idxs, obj_idxs, symbol_idxs = dict(), dict(), dict()

    # Declare language sorts
    for sort in language.sorts:
        if isinstance(sort, tsk.Interval):
            if sort.name == 'Real':
                tid = info.add_fstype(sort.name, cext.type_id.float_t)
                type_idxs[sort] = tid
                continue
            elif sort.name == 'Integer' or sort.name == 'Natural':
                tid = info.add_fstype(sort.name, cext.type_id.int_t)
                type_idxs[sort] = tid
                continue
            # TODO TODO TODO
            # assert 0, "To implement"
            print("\n\nWARNING: WE'RE IGNORING INTERVAL TYPES AT THE MOMENT. FIX THIS.\n\n")
            pass

        elif isinstance(sort, tsk.Sort):
            tid = info.add_fstype(sort.name, cext.type_id.object_t)
            type_idxs[sort] = tid
        else:
            raise RuntimeError("Unknown sort type: {}".format(sort))

    # Declare language objects
    for o in language.constants():
        oid = info.add_object(o.symbol, type_idxs[o.sort])
        obj_idxs[o] = oid

        # TEST
        print("get_object_name({}): {}".format(oid, info.get_object_name(oid)))

    # Declare language symbols
    for p in util.get_symbols(language, include_builtin=True):
        signature = [type_idxs[t] for t in p.sort]
        sid = cext.add_symbol_to_language_info(str(p.name), get_fs_symbol_t(p), signature, info)
        print("Symbol {} registered with ID {} ({})".format(p, sid, info.get_symbol_name(sid)))
        symbol_idxs[p] = sid

    return LanguageInfoWrapper(info, type_idxs, obj_idxs, symbol_idxs)


def translate_problem(problem):
    assert isinstance(problem, tarski.fstrips.Problem)
    info_wrapper = create_language_info(problem.language)
    translator = FSTRIPSTranslator(info_wrapper)

    # Translate goal
    goal = translator.translate_formula(problem.goal, tsk.VariableBinding.empty())
    logging.info("Translated goal formula: {}".format(goal.print(info_wrapper.linfo)))

    # Translate action schemas
    actions = [translator.translate_action(id_, a) for id_, (name, a) in enumerate(problem.actions.items())]

    # TODO Translate initial state
    init = cext.Interpretation()

    return cext.create_problem(problem.name, problem.domain_name, actions, init, goal)


class FSTRIPSTranslator:
    def __init__(self, language_info_wrapper):
        self.language_info_wrapper = language_info_wrapper

    def to_string(self, element):
        assert isinstance(element, (cext.LogicalElement, cext.AtomicEffect))
        return element.print(self.language_info_wrapper.linfo)

    def get_symbol_id(self, symbol):
        return self.language_info_wrapper.symbol_idxs[symbol]

    def get_type_id(self, sort):
        return self.language_info_wrapper.type_idxs[sort]

    def get_object_id(self, obj):
        return self.language_info_wrapper.obj_idxs[obj]
        # return cext.make_object(True)

    def translate_term(self, term, binding):
        assert isinstance(term, tsk.Term)
        if isinstance(term, tsk.Variable):
            varid = binding.index(term.name)
            typeid = self.get_type_id(term.sort)
            return cext.LogicalVariable(varid, term.name, typeid)

        elif isinstance(term, tsk.CompoundTerm):
            symbol_id = self.get_symbol_id(term.name)
            return cext.create_composite_term(symbol_id, self.translate_term_list(term.subterms, binding))

        elif isinstance(term, tsk.Constant):
            oid = self.get_object_id(term)
            typeid = self.get_type_id(term.sort)
            return cext.Constant(oid, typeid)

        raise RuntimeError("Unexpected Tarski element type: {}".format(term))

    def translate_term_list(self, terms, binding):
        return [self.translate_term(t, binding) for t in terms]

    def translate_formula(self, formula, binding):
        assert isinstance(formula, tsk.Formula)

        if isinstance(formula, tsk.Tautology):
            return cext.Tautology()

        elif isinstance(formula, tsk.Contradiction):
            return cext.Contradiction()

        elif isinstance(formula, tsk.Atom):
            symbol_id = self.get_symbol_id(formula.head)
            subterms = self.translate_term_list(formula.subterms, binding)
            return cext.create_atomic_formula(symbol_id, subterms)
            # return cext.AtomicFormula(symbol, subterms)

        elif isinstance(formula, tsk.CompoundFormula):
            # print("Translating compound formula {}...".format(formula))
            connective = self.translate_connective(formula.connective)
            subformulas = self.translate_formula_list(formula.subformulas, binding)
            translated = cext.create_composite_formula(connective, subformulas)
            # print("Finished translating compound formula {}".format(formula))
            print("Result of translation of compound formula '{}': {}".format(formula, self.to_string(translated)))
            return translated

        elif isinstance(formula, tsk.QuantifiedFormula):
            raise RuntimeError("TO IMPLEMENT :-)")
            # vars_ = print_variable_list(formula.variables)
            # e.g. (exists (?x - object) (and (= ?x 2)))
            # return '({} ({}) {})'.format(formula.quantifier, vars_, print_formula(formula.formula))

        raise RuntimeError("Unexpected Tarski element type: {}".format(formula))

    def translate_connective(self, connective):
        return {
            tsk.Connective.And: cext.Connective.And,
            tsk.Connective.Or: cext.Connective.Or,
            tsk.Connective.Not: cext.Connective.Not,
        }[connective]

    def translate_quantifier(self, connective):
        return {
            tsk.Quantifier.Exists: cext.Quantifier.Exists,
            tsk.Quantifier.Forall: cext.Quantifier.Forall,
        }[connective]

    def translate_formula_list(self, formulas, binding):
        return [self.translate_formula(f, binding) for f in formulas]

    def translate_action(self, id_, action):
        # Order matters: the binding unit needs to be created when the effects are processed
        params = action.parameters.vars()
        signature = [self.get_type_id(v.sort) for v in params]
        parameter_names = [v.name for v in params]
        precondition = self.translate_formula(action.precondition, action.parameters)
        effects = [self.translate_effect(eff, action.parameters) for eff in action.effects]
        # print(self.to_string(effects[0]))
        return cext.create_action_schema(id_, action.name, signature, parameter_names, precondition, effects)

    def translate_effect(self, eff, binding):
        if isinstance(eff, (AddEffect, DelEffect)):
            atom = self.translate_formula(eff.atom, binding)
            condition = self.translate_formula(eff.condition, binding)
            # print("Effect atom translation '{}': {}".format(eff.atom, self.to_string(atom)))
            type_ = cext.AtomicEffectType.Del if isinstance(eff, DelEffect) else cext.AtomicEffectType.Add
            translated = cext.create_atomic_effect(atom, type_, condition)
            print("Effect translation '{}': {}".format(eff, self.to_string(translated)))
            return translated

        raise RuntimeError("Unknown effect type")


def get_fs_symbol_t(s):
    """ Return the FS symbol_t type corresponding to the given predicate function """
    assert isinstance(s, (tsk.PredicateSymbol, tsk.FunctionSymbol))
    return cext.symbol_t.Function if isinstance(s, tsk.FunctionSymbol) else cext.symbol_t.Predicate
