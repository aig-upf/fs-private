"""
 This tests the parsing of particular FSTRIPS language elements and its
 mapping into appropriate JSON data structures.
"""

from .common import process_types, generate_objects, MockTask

# This should be imported from a custom-set PYTHONPATH containing the path to Fast Downward's PDDL parser
from pddl.conditions import Conjunction, Atom
from compilation.formula_processor import FormulaProcessor


def unwrap_conjunction(element):
    assert element['conditions']['type'] == 'conjunction'
    return element['conditions']['elements']


def unwrap_atom(element):
    assert element['type'] == 'atom'
    return element['symbol'], element['negated'], element['elements']


def generate_base_bw_instance():
    types, supertypes = process_types(dict(block='object'))
    objects = generate_objects(dict(b1='block', b2='block'))
    return MockTask(types=types, supertypes=supertypes, objects=objects, fluent_symbols=['on'])


def test_predicate():
    task = generate_base_bw_instance()

    conjunction = Conjunction([Atom('on', ['b1', 'b2'])])
    processed = FormulaProcessor(task, conjunction).process()

    elements = unwrap_conjunction(processed)
    assert len(elements) == 1

    symbol, negated, elements = unwrap_atom(elements[0])
    assert symbol == 'on' and len(elements) == 2 and all(elem['type'] == 'constant' for elem in elements)


def test_conjunction():
    assert False


def test_function():
    assert False


def test_existential():
    assert False
