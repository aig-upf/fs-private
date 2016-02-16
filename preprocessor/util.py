import os
import sys
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


def is_action_parameter(name):
    return isinstance(name, str) and name[0] == '?'


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
