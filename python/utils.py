
import os
import sys
import errno


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


def save_file(name, content):
    with open(name, "w") as f:
        f.write(content)
