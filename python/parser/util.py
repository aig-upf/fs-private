"""
    Some basic utility methods.
"""
import os
import re
import unicodedata
from collections import OrderedDict


def normalize(name):
    slug = unicodedata.normalize('NFKD', name)
    slug = slug.encode('ascii', 'ignore').lower()
    slug = re.sub(r'[^a-z0-9]+', '_', slug.decode()).strip('-')
    slug = re.sub(r'[-]+', '_', slug)
    return slug


def normalize_and_camelcase(name):
    return to_camelcase(normalize(name))


def normalize_action_name(name):
    return normalize_and_camelcase(name) + 'Action'


def to_camelcase(word):
    return ''.join(x.capitalize() or '_' for x in word.split('_'))


def load_file(filename):
    with open(filename, 'r') as f:
        c = f.read()
    return c


def load_file_safely(filename):
    if not os.path.isfile(filename):
        return None

    with open(filename, 'r') as f:
        c = f.read()
    return c


def bool_string(value):
    return 'true' if value else 'false'


def is_external(symbol):
    return symbol[0] == '@'


def has_unbounded_arity(symbol):
    return symbol[0:2] == '@@'
