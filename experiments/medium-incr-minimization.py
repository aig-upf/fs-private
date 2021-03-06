#! /usr/bin/env python
import itertools
import os

from lab.experiment import Experiment
from downward import suites
from common import generate_environment, get_fsplanner_binary, add_all_runs, \
    filter_benchmarks_if_test_run

TIME_LIMIT = 3600
MEMORY_LIMIT = 16000

SUITE = [
    'blocks',
    'sokoban-opt08-strips',
    'visitall-opt11-strips',
    'pipesworld-tankage',
    # 'floortile-opt11-strips',
    # 'hiking-opt14-strips',
    # 'organic-synthesis-opt18-strips',
]

BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]


def algorithms():
    # We use Lab's data directory as workspace.
    lifted_nocustom_asp = [get_fsplanner_binary(), '--domain', '{domain}', '-i', '{problem}', '--sdd', '--driver', 'bfs-sdd',
                              '--output', '.', "--options", "verbose_stats=true,sdd.minimization_time=600,sdd.custom_me=false", "--sdd_with_reachability"]

    lifted_nocustom_asp_incr = [get_fsplanner_binary(), '--domain', '{domain}', '-i', '{problem}', '--sdd', '--driver', 'bfs-sdd',
                           '--output', '.', "--options", "verbose_stats=true,sdd.minimization_time=600,sdd.custom_me=false", "--sdd_with_reachability", "--sdd_incr_minimization_time", "5"]

    lifted_nocustom_vo_asp = [get_fsplanner_binary(), '--domain', '{domain}', '-i', '{problem}', '--sdd', '--driver', 'bfs-sdd',
                           '--output', '.', "--options", "verbose_stats=true,sdd.minimization_time=600,sdd.custom_me=false",  "--var_ordering", "arity", "--sdd_with_reachability"]

    # We don't use the ASP reachability analysis at the moment to be as close as possible as the above experiment
    grounded = [get_fsplanner_binary(), '--domain', '{domain}', '-i', '{problem}', '--driver', 'bfs',
                '--output', '.', "--options", "verbose_stats=true,successor_generation=naive"]

    grounded_asp = [get_fsplanner_binary(), '--domain', '{domain}', '-i', '{problem}', '--driver', 'bfs',
                '--output', '.', "--options", "verbose_stats=true,successor_generation=naive"]

    # match_tree = [get_fsplanner_binary(), '--domain', '{domain}', '-i', '{problem}', '--driver', 'bfs',
    #               '--output', '.', "--options", "verbose_stats=true,successor_generation=match_tree"]

    return {
        # 'brfs-naive': grounded,
        'brfs-sdd-asp_incr': lifted_nocustom_asp_incr,
        'brfs-sdd-asp': lifted_nocustom_asp,
    }


def main():
    environment = generate_environment(email="guillem.frances@upf.edu",
                                       memory_limit=MEMORY_LIMIT, time_limit=TIME_LIMIT)
    exp = Experiment(environment=environment)

    benchmarks = suites.build_suite(os.environ["DOWNWARD_BENCHMARKS"], SUITE)
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
