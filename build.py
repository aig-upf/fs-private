#!/usr/bin/env python

"""
    A helper build script - Builds the production library and vanilla solver
"""
import argparse
import os
import shutil
import subprocess
import sys
import multiprocessing


def create_parser():
    parser = argparse.ArgumentParser(description='FS Planner Build Script')
    parser.add_argument('-a', '--all', action='store_true', help="Build all releases.")
    parser.add_argument('-p', '--prod', action='store_true', help="Build the production release.")
    parser.add_argument('-d', '--debug', action='store_true', help="Build the debug release.")
    parser.add_argument('-e', '--edebug', action='store_true', help="Build the extreme-debug release.")
    return parser


def single_build(directory, command):

    # Build the FS planner production library
    print("\nBuilding FS executable: \"{}\" @ {}".format(command, directory))
    sys.stdout.flush()
    output = subprocess.call(command.split(), cwd=directory)
    if output:
        sys.exit(output)

    # Move the planner to the "generic" directory, for backwards compatibility
    planner_dir = os.path.join(directory, 'planners', 'generic')
    for file in os.listdir(directory):
        if file.endswith(".bin"):
            shutil.copy2(os.path.join(directory, file), os.path.join(planner_dir, file))
    print('')


def get_command(cpus, debug):
    return 'scons -j {} {}'.format(cpus, debug)


def main(parser, args):
    args = parser.parse_args(args)
    flags = dict(prod="", debug="debug=1", edebug="edebug=1")

    # Check that at least some build target was specified
    all_attrs = ["all"] + list(flags.keys())
    if not any(getattr(args, attr) for attr in all_attrs):
        parser.print_help()
        sys.exit(1)

    current_dir = os.path.dirname(os.path.abspath(__file__))
    cpus = min(5, multiprocessing.cpu_count())
    print("Starting build on directory '{}' with {} CPUs.".format(current_dir, cpus))

    if args.all:  # We'll build all releases
        [setattr(args, k, True) for k in flags.keys()]

    required_flags = [v for k, v in flags.items() if getattr(args, k, False)]

    for flag in required_flags:
        single_build(current_dir, get_command(cpus, flag))


if __name__ == "__main__":
    main(create_parser(), sys.argv[1:])
