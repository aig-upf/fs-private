
import fnmatch
import os


def which(program):
    """ Helper function emulating unix 'which' command """
    for path in os.environ["PATH"].split(os.pathsep):
        path = path.strip('"')
        exe_file = os.path.join(path, program)
        if os.path.isfile(exe_file) and os.access(exe_file, os.X_OK):
            return exe_file
    return None


def locate_source_files(base_dir, pattern, fromdir='src'):
    matches = []
    for root, dirnames, filenames in os.walk(base_dir):
        if "__" in root: continue  # Don't process directories with double underscore on their names
        for filename in fnmatch.filter(filenames, pattern):
            matches.append(os.path.join(root[root.find(fromdir):], filename))

    return matches
