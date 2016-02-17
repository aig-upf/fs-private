"""
 This tests the parsing of particular object types
"""
import pytest

from .common import process_types


def test_simple_type():
    types, supertypes = process_types([('block', 'object'), ('object', None)])
    assert len(supertypes) == len(types) == 4, "Types should be: object, bool, int and block"
    assert 'object' in supertypes and not supertypes['object']
    assert supertypes['block'] == ['object']


def test_hierarchy():
    types, supertypes = process_types([('dad', 'grandpa'), ('object', None), ('grandpa', 'object'), ('me', 'dad')])
    assert len(supertypes) == len(types) == 6
    assert len(supertypes['me']) == 3 and len(supertypes['dad']) == 2 and len(supertypes['grandpa']) == 1


def test_no_object_type_declared():
    check = "Even if not explicitly declared, the base 'object', 'bool' and 'int' types get properly identified"
    types, supertypes = process_types([])
    assert len(supertypes) == len(types) == 3, check
    assert 'object' in supertypes and '_bool_' in supertypes and not supertypes['object'], check


def test_duplicate_declaration():
    with pytest.raises(RuntimeError):
        process_types([('object', None), ('block', 'object'), ('block', 'int')])
