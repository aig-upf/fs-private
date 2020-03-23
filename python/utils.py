
import os
import sys
import errno


def fix_seed_and_possibly_rerun(verbose=False):
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
    if verbose:
        print("Fixing PYTHONHASHSEED to 1 for reproducibility", flush=True)
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


def mkdirp(directory):
    """" mkdir -p -like functionality """
    if not os.path.isdir(directory):
        os.makedirs(directory)


def silentremove(filename):
    """ Try to remove the given filename, but don't complain if it does not exist. """
    try:
        os.remove(filename)
    except OSError as e:
        if e.errno != errno.ENOENT:  # errno.ENOENT = no such file or directory
            raise  # re-raise exception if a different error occured


def save_file(name, content):
    with open(name, "w") as f:
        f.write(content)


def is_int(s):
    if isinstance(s, float):
        return False
    try:
        int(s)
        return True
    except (ValueError, TypeError):
        return False


def is_float(s):
    if isinstance(s, int):
        return False
    try:
        float(s)
        return True
    except (ValueError, TypeError):
        return False
