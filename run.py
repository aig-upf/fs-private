#!/usr/bin/env python3
"""
The main entry point to the FS planner.

Load the FS parser and bootstrap the whole parsing + (optionally) solving process.
You can see the different options by invoking this script with the `-h` option.
"""

import sys

from python import utils, runner

if __name__ == "__main__":
    # Make sure that the random seed is fixed before running the script, to ensure determinism
    # in the output of the parser.
    if not utils.fix_seed_and_possibly_rerun(verbose="--debug" in sys.argv):
        print('Invocation command: "{}"'.format(' '.join(sys.argv)))
        runner.main(sys.argv[1:])
