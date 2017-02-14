"""
    Some basic utility methods.
"""
import os
import errno
import sys
import unicodedata
import re
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


def mkdirp(directory):
    """" mkdir -p -like functionality """
    if not os.path.isdir(directory):
        os.makedirs(directory)


def silentremove(filename):
    try:
        os.remove(filename)
    except OSError as e:
        if e.errno != errno.ENOENT:  # errno.ENOENT = no such file or directory
            raise  # re-raise exception if a different error occured


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
    return 'true' if value else 'false'


def fix_seed_and_possibly_rerun():
    """
    Make sure the environment variable `PYTHONHASHSEED` is set to 1 so that the order of some of the problem's
    components, which is determined by iterating a Python dictionary, is always consistently the same.

    To do so, (@see http://stackoverflow.com/a/25684784), this method might have to spawn a subprocess
    which is identical to the current process in everything but in its set of environment variables,
    in which case it will return True.

    :return: True iff a new subprocess mirroring the current one was executed.
    """
    # Base case: Seed has already been fixed, so we simply return False to signal that execution can carry on normally
    if get_seed() == 1:
        return False

    # Otherwise we print a warning and re-run the process with a fixed hash seed envvar.
    print('\n' + "*" * 80)
    print("WARNING! Fixing PYTHONHASHSEED to 1 to obtain more reliable results")
    print("*" * 80 + '\n')
    # We simply set the environment variable and re-call ourselves.
    import subprocess
    env = dict(os.environ)
    env.update(PYTHONHASHSEED='1')
    subprocess.call(["python3"] + sys.argv, env=env)
    return True


def get_seed():
    try:
        return int(os.environ['PYTHONHASHSEED'])
    except KeyError as _:
        return None


def is_external(symbol):
    return symbol[0] == '@'


class UninitializedAttribute(object):
    def __init__(self, name):
        object.__setattr__(self, 'name', name)

    def _raise_error(self, *args, **kwargs):
        raise AttributeError("Attempt to access object '{}', which has not yet been initialized. "
                             "Revise the application workflow.".format(object.__getattribute__(self, 'name')))

    __getattr__ = _raise_error
    __setattr__ = _raise_error
    __delattr__ = _raise_error
    __len__ = _raise_error
    __getitem__ = _raise_error
    __setitem__ = _raise_error
    __delitem__ = _raise_error
    __iter__ = _raise_error
    __contains__ = _raise_error


class IndexDictionary(object):
    """
    A very basic indexing mechanism object that assigns consecutive indexes to the indexed objects.
    """
    def __init__(self, elements=None):
        self.data = OrderedDict()
        self.objects = []
        elements = [] if elements is None else elements
        for element in elements:
            self.add(element)

    def get_index(self, key):
        return self.data[key]

    def get_object(self, index):
        return self.objects[index]

    def add(self, obj):
        if obj in self.data:
            raise RuntimeError("Duplicate element '{}'".format(obj))
        self.data[obj] = len(self.data)
        self.objects.append(obj)

    def dump(self):
        return [str(o) for o in self.data.keys()]

    def __str__(self):
        return ','.join('{}: {}'.format(k, o) for k, o in self.data.items())

    __repr__ = __str__

    def __iter__(self):
        return self.data.__iter__()

    def __contains__(self, k):
        return k in self.data

    def __len__(self):
        return len(self.data)
