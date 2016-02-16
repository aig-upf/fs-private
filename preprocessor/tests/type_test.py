"""
 This tests the parsing of particular object types
"""
from .common import process_types


def test_simple_type():
    types, supertypes = process_types(dict(object=None, block='object'))
    assert len(supertypes) == 3 and len(types) == 3, "Types should be: object, bool and block"
    assert 'object' in supertypes and not supertypes['object']
    assert supertypes['block'] == ['object']


def test_hierarchy():
    types, supertypes = process_types(dict(object=None, grandpa='object', dad='grandpa', me='dad'))
    assert len(supertypes) == 5 and len(types) == 5
    assert len(supertypes['me']) == 3 and len(supertypes['dad']) == 2 and len(supertypes['grandpa']) == 1


def test_no_object_declared():
    check = "Even if not explicitly declared, the base 'object' and 'bool' types get properly identified"
    types, supertypes = process_types(dict(block='object'))
    assert len(supertypes) == 3 and len(types) == 3, check
    assert 'object' in supertypes and '_bool_' in supertypes and not supertypes['object'], check
