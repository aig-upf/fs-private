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
    'blocks:probBLOCKS-4-0.pddl',
    'blocks:probBLOCKS-6-0.pddl',
    'blocks:probBLOCKS-8-0.pddl',
    'blocks:probBLOCKS-10-0.pddl',
    'blocks:probBLOCKS-14-0.pddl',
    'blocks:probBLOCKS-17-0.pddl',

    # 'organic-synthesis-opt18-strips:p01.pddl',
    # 'organic-synthesis-opt18-strips:p02.pddl',
    # 'blocks',
    # 'sokoban-opt08-strips',

    # 'blocks:probBLOCKS-10-0.pddl',
    # 'pipesworld-tankage',
    # 'pipesworld-tankage:p02-net1-b6-g4-t50.pddl',
    # 'pipesworld-tankage:p03-net1-b8-g3-t80.pddl',
]

BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]


def algorithms():
    # We use Lab's data directory as workspace.
    lifted = [get_fsplanner_binary(), '--domain', '{domain}', '-i', '{problem}', '--sdd', '--driver', 'bfs-sdd',
              '--output', '.', "--options", "verbose_stats=true,sdd.minimization_time=600,sdd.custom_me=true"]

    # We don't use the ASP reachability analysis at the moment to be as close as possible as the above experiment
    grounded = [get_fsplanner_binary(), '--domain', '{domain}', '-i', '{problem}', '--driver', 'bfs',
                '--output', '.', "--options", "verbose_stats=true,successor_generation=naive"]

    match_tree = [get_fsplanner_binary(), '--domain', '{domain}', '-i', '{problem}', '--driver', 'bfs',
                  '--output', '.', "--options", "verbose_stats=true,successor_generation=match_tree"]

    # configs = dict()
    # for min_time, custom_me in itertools.product(['0', '100'], [True, False]):
    #     name = f'brfs-sdd-{min_time}' + ('-custom_me' if custom_me else '')
    #     configs[name] = lifted + ['--options']

    # return {'brfs-sdd': lifted}  # For testing purposes
    return {'brfs-sdd': lifted, 'brfs-ground-enum': grounded, 'brfs-ground-mt': match_tree}


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
