"""
    Translation of Tarski planning problems into the FS planner native format
"""
import logging

import tarski
import tarski.syntax as tsk
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
            # assert 0, "To implement"
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

    # Declare language symbols
    for p in util.get_symbols(language, include_builtin=False):
        signature = [type_idxs[t] for t in p.sort]
        sid = cext.add_symbol_to_language_info(p.symbol, get_fs_symbol_t(p), signature, info)
        symbol_idxs[p] = sid

    return LanguageInfoWrapper(info, type_idxs, obj_idxs, symbol_idxs)



def translate_problem(problem):
    assert isinstance(problem, tarski.fstrips.Problem)
    info_wrapper = create_language_info(problem.language)
    translator = FormulaTranslator(info_wrapper)
    goal = translator.translate_formula(problem.goal)
    print("Let's do it")
    s = goal.print(info_wrapper.linfo)
    print("Translated goal formula: {}".format(s))
    return None, None, None, None, goal


def get_symbol_id(symbol):
    # TODO DERIVE NUMERIC ID FROM SYMBOL NAME
    return 9999


def get_variable_id(symbol):
    # TODO
    return 9999


def get_type_id(sort):
    # TODO
    return 9999


def get_object_id(symbol):
    # TODO
    return cext.make_object(True)


class FormulaTranslator(object):
    def __init__(self, language_info_wrapper):
        self.language_info_wrapper = language_info_wrapper

    def to_string(self, element):
        assert isinstance(element, cext.LogicalElement)
        return element.print(self.language_info_wrapper.linfo)

    def translate_formula(self, formula):
        assert isinstance(formula, tsk.Formula)

        if isinstance(formula, tsk.Tautology):
            return cext.Tautology()

        elif isinstance(formula, tsk.Contradiction):
            return cext.Contradiction()

        elif isinstance(formula, tsk.Atom):
            symbol = get_symbol_id(formula.predicate.symbol)
            subterms = self.translate_term_list(formula.subterms)
            return cext.create_atomic_formula(symbol, subterms)
            # return cext.AtomicFormula(symbol, subterms)

        elif isinstance(formula, tsk.CompoundFormula):
            print("Translating compound formula {}...".format(formula))
            connective = self.translate_connective(formula.connective)
            subformulas = self.translate_formula_list(formula.subformulas)
            translated = cext.create_composite_formula(connective, subformulas)
            print("Finished translating compound formula {}".format(formula))
            print("Resulting formula: {}".format(self.to_string(translated)))
            return translated

        elif isinstance(formula, tsk.QuantifiedFormula):
            raise RuntimeError("TO IMPLEMENT :-)")
            # vars_ = print_variable_list(formula.variables)
            # e.g. (exists (?x - object) (and (= ?x 2)))
            # return '({} ({}) {})'.format(formula.quantifier, vars_, print_formula(formula.formula))

        raise RuntimeError("Unexpected Tarski element type: {}".format(formula))

    def translate_term(self, term):
        assert isinstance(term, tsk.Term)
        if isinstance(term, tsk.Variable):
            varid = get_variable_id(term.symbol)
            typeid = get_type_id(term.sort)
            return cext.LogicalVariable(varid, term.symbol, typeid)

        elif isinstance(term, tsk.CompoundTerm):
            symbol_id = get_symbol_id(term.symbol.symbol)
            return cext.CompositeTerm(symbol_id, self.translate_term_list(term.subterms))

        elif isinstance(term, tsk.Constant):
            oid = get_object_id(term.symbol)
            typeid = get_type_id(term.sort)
            return cext.Constant(oid, typeid)

        raise RuntimeError("Unexpected Tarski element type: {}".format(term))

    def translate_term_list(self, terms):
        return [self.translate_term(t) for t in terms]

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

    def translate_formula_list(self, formulas):
        return [self.translate_formula(f) for f in formulas]


def get_fs_symbol_t(s):
    """ Return the FS symbol_t type corresponding to the given predicate function """
    assert isinstance(s, (tsk.Predicate, tsk.Function))
    return cext.symbol_t.Function if isinstance(s, tsk.Function) else cext.symbol_t.Predicate
