#!/usr/bin/python3

"""
    A helper build script - Builds the production library and vanilla solver
"""
import argparse
import os
import subprocess
import sys
import multiprocessing


def parse_arguments(args):
    parser = argparse.ArgumentParser(description='FS Planner Build Script')
    parser.add_argument('-a', '--all', action='store_true', help="Build all releases.")
    parser.add_argument('-p', '--prod', action='store_true', help="Build the production release.")
    parser.add_argument('-d', '--debug', action='store_true', help="Build the debug release.")
    parser.add_argument('-e', '--edebug', action='store_true', help="Build the extreme-debug release.")
    return parser.parse_args(args)


def single_build(directory, command):
    # First build the LAPKT production library
    # lapkt_dir = os.environ['LAPKT2_PATH']
    # print('\n')
    # print("Building LAPKT library...")
    # sys.stdout.flush()
    # output = subprocess.call(command.split(), cwd=lapkt_dir)
    # if output:
    # sys.exit(output)

    # Build the FS planner production library
    print('\n')
    print("Building FS library...")
    sys.stdout.flush()
    output = subprocess.call(command.split(), cwd=directory)
    if output:
        sys.exit(output)

    # Build the vanilla generic FS planner
    generic_planner_dir = os.path.join(directory, 'planners', 'generic')
    print('\n')
    print("Building FS vanilla planner...")
    sys.stdout.flush()
    output = subprocess.call(command.split(), cwd=generic_planner_dir)
    if output:
        sys.exit(output)

    print('\n')


def get_command(cpus, debug):
    return 'scons -j {} {}'.format(cpus, debug)


def main(args):
    flags = dict(prod="", debug="debug=1", edebug="edebug=1")
    current_dir = os.path.dirname(os.path.abspath(__file__))
    cpus = min(5, multiprocessing.cpu_count())
    print("Starting build on directory '{}' with {} CPUs.".format(current_dir, cpus))

    if args.all:  # We'll build all releases
        [setattr(args, k, True) for k in flags.keys()]

    required_flags = [v for k, v in flags.items() if getattr(args, k, False)]

    for flag in required_flags:
        single_build(current_dir, get_command(cpus, flag))


if __name__ == "__main__":
    main(parse_arguments(sys.argv[1:]))
