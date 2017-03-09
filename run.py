#!/usr/bin/env python3
"""
The main entry point to the FS planner.

Load the FS parser and bootstrap the whole parsing + (optionally) solving process.
"""

import sys

from python import utils, runner

if __name__ == "__main__":
    # Run only if the hash seed has been set
    if not utils.fix_seed_and_possibly_rerun():
        runner.main(sys.argv[1:])
