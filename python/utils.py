
import os
import sys
import errno


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
