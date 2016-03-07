"""
 This tests the type-handling module
"""
import pytest

from object_types import process_type_hierarchy, process_problem_types
from .common import process_types, generate_fd_objects, generate_fd_types, generate_fd_bound


def test_empty_type():
    types, type_map = process_types([('block', 'object')], [])
    assert type_map['block'] == [] and len(type_map) == len(types) == 4, \
        "A type with no objects still needs to be on the type_map"


def test_simple_type():
    fd_objects = generate_fd_objects([('b1', 'block'), ('b2', 'block')])
    types, type_map = process_types([('block', 'object'), ('object', None)], fd_objects)
    assert len(type_map) == len(types) == 4, "Types should be: object, bool, int and block"
    assert type_map['object'] == type_map['block'] == ['b1', 'b2']


def test_int_type():
    types, type_map = process_problem_types([], [], [generate_fd_bound(('val', 5, 15))])
    assert type_map['val'] == list(range(5, 16))

    with pytest.raises(RuntimeError):
        process_problem_types([], [], [generate_fd_bound(('val', 10, 1))])  # The bound is incorrect


def test_hierarchy():
    fd_objects = generate_fd_objects([('b1', 'me'), ('b2', 'dad'), ('b3', 'grand'), ('b4', 'object')])
    types, t_map = process_types([('dad', 'grand'), ('object', None), ('grand', 'object'), ('me', 'dad')], fd_objects)
    assert len(t_map) == len(types) == 6
    assert len(t_map['me']) == 1 and len(t_map['dad']) == 2 and len(t_map['grand']) == 3 and len(t_map['object']) == 4


def test_intermediate_parent_map():
    fd_types = generate_fd_types([('dad', 'grand'), ('object', None), ('grand', 'object'), ('me', 'dad')])
    types, supertypes = process_type_hierarchy(fd_types)
    assert len(supertypes) == len(types) == len(fd_types) + 2  # the declared types plus bool, int.
    assert len(supertypes['me']) == 3 and len(supertypes['dad']) == 2 and len(supertypes['grand']) == 1


def test_no_object_types():
    check = "Even if not explicitly declared, the base 'object', 'bool' and 'int' types get properly identified"
    types, type_map = process_types([], [])
    assert len(type_map) == len(types) == 3, check
    assert type_map['object'] == type_map['int'] == [], check
    assert type_map['bool'] == ['false', 'true'], "The Bool type has always exactly the two expected objects"


def test_duplicate_declaration():
    with pytest.raises(RuntimeError):
        process_types([('object', None), ('block', 'object'), ('block', 'int')], [])
