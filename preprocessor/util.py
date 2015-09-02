import os
import unicodedata
import re


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


def make_type_varname(obj):
    return normalize(obj) + '_t'


def make_extension_varname(extension):
    return normalize(filter_extension_name(extension)) + '_ext'


def make_extension_idxname(extension):
    return make_extension_varname(filter_extension_name(extension)) + '_idx'


def make_param_name(param):
    return normalize(param[1:]) + '_idx'  # we get rid of the '?' in the variable name


def is_action_parameter(name):
    return isinstance(name, str) and name[0] == '?'


def filter_extension_name(extension):
    # We substitute the predicate '=' for a syntactically acceptable predicate '_eq_'
    return '_eq_' if extension == '=' else extension


def mkdirp(directory):
    """" mkdir -p -like functionality """
    if not os.path.isdir(directory):
        os.makedirs(directory)


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


def save_file(name, content):
    with open(name, "w") as f:
        f.write(content)


def is_int(s):
    try:
        int(s)
        return True
    except ValueError:
        return False


def bool_string(value):
    return '_true_' if value else '_false_'