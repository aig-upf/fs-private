"""
 This tests the parsing of particular object types
"""

from pddl import Type
from pddl.pddl_types import set_supertypes
from taskgen import process_type_hierarchy


def process_case(types):
    processed = [Type(name, parent) for name, parent in types.items()]
    set_supertypes(processed)
    return process_type_hierarchy(processed)


def test_type():
    types, supertypes = process_case(dict(object=None, block='object'))
    assert len(supertypes) == 2 and len(types) == 2
    assert 'object' in supertypes and not supertypes['object']
    assert supertypes['block'] == ['object']


def test_hierarchy():
    types, supertypes = process_case(dict(object=None, grandpa='object', dad='grandpa', me='dad'))
    assert len(supertypes) == 4 and len(types) == 4
    assert len(supertypes['me']) == 3 and len(supertypes['dad']) == 2 and len(supertypes['grandpa']) == 1


def test_no_object_declared():
    check = "Even if not explicitly declared, the base 'object' type gets properly identified"
    types, supertypes = process_case(dict(block='object'))
    assert len(supertypes) == 2 and len(types) == 2, check
    assert 'object' in supertypes and not supertypes['object'], check
