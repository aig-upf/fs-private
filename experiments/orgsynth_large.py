#! /usr/bin/env python
import itertools
import os
import pathlib

from lab.experiment import Experiment
from downward import suites
from common import generate_environment, get_fsplanner_binary, add_all_runs, \
    filter_benchmarks_if_test_run

TIME_LIMIT = 7200
MEMORY_LIMIT = 16000

SUITE = [
    'organic-synthesis',
]

ORGSYNTH_ORIGINAL_DOMAIN = pathlib.Path.home() / "projects/code/organic-synthesis-benchmarks"


def algorithms():
    # We use Lab's data directory as workspace.
    lifted_nocustom = [get_fsplanner_binary(), '--domain', '{domain}', '-i', '{problem}', '--sdd', '--driver', 'bfs-sdd',
                       '--output', '.', "--options", "verbose_stats=true,sdd.minimization_time=600,sdd.custom_me=false"]

    lifted_nocustom_vo = [get_fsplanner_binary(), '--domain', '{domain}', '-i', '{problem}', '--sdd', '--driver', 'bfs-sdd',
                      '--output', '.', "--options", "verbose_stats=true,sdd.minimization_time=600,sdd.custom_me=false", "--var_ordering", "arity"]

    lifted_nocustom_asp = [get_fsplanner_binary(), '--domain', '{domain}', '-i', '{problem}', '--sdd', '--driver', 'bfs-sdd',
                              '--output', '.', "--options", "verbose_stats=true,sdd.minimization_time=600,sdd.custom_me=false", "--sdd_with_reachability"]

    lifted_nocustom_vo_asp = [get_fsplanner_binary(), '--domain', '{domain}', '-i', '{problem}', '--sdd', '--driver', 'bfs-sdd',
                           '--output', '.', "--options", "verbose_stats=true,sdd.minimization_time=600,sdd.custom_me=false",  "--var_ordering", "arity", "--sdd_with_reachability"]

    return {'brfs-sdd-old': lifted_nocustom,
            'brfs-sdd-vo': lifted_nocustom_vo,
            'brfs-sdd-asp': lifted_nocustom_asp,
            'brfs-sdd-vo-asp': lifted_nocustom_vo_asp}


def main():
    environment = generate_environment(email="guillem.frances@upf.edu",
                                       memory_limit=MEMORY_LIMIT, time_limit=TIME_LIMIT)
    exp = Experiment(environment=environment)

    benchmarks = suites.build_suite(ORGSYNTH_ORIGINAL_DOMAIN, SUITE)
    benchmarks = filter_benchmarks_if_test_run(benchmarks)

    add_all_runs(
        experiment=exp,
        suites=benchmarks,
        algorithms=algorithms(),
        time_limit=TIME_LIMIT,
        memory_limit=MEMORY_LIMIT)

    # Parse the commandline and run the specified steps.
    exp.run_steps()


if __name__ == '__main__':
    main()
