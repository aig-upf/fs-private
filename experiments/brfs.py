#! /usr/bin/env python


import os

from lab.experiment import Experiment
from downward import suites
from common import generate_environment, get_fsplanner_binary, generate_benchmark_suite, add_all_runs

TIME_LIMIT = 1800
MEMORY_LIMIT = 64000

SUITE = [
    'blocks',
    'organic-synthesis-opt18-strips',
    'pipesworld-tankage',
    'pipesworld-notankage',
]

BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]


def algorithms():
    # We use Lab's data directory as workspace.
    lifted = [get_fsplanner_binary(), '--domain', '{domain}', '-i', '{problem}', '--sdd', '--driver', 'bfs-sdd',
              '--output', '.']

    grounded = [get_fsplanner_binary(), '--domain', '{domain}', '-i', '{problem}', '--sdd', '--driver', 'bfs',
                '--output', '.']

    return {'brfs-sdd': lifted, 'brfs-ground': grounded}


def main():
    environment = generate_environment(email="guillem.frances@upf.edu",
                                       memory_limit=MEMORY_LIMIT, time_limit=TIME_LIMIT)
    exp = Experiment(environment=environment)

    suite = generate_benchmark_suite(SUITE)

    add_all_runs(
        experiment=exp,
        suites=suites.build_suite(BENCHMARKS_DIR, suite),
        algorithms=algorithms(),
        time_limit=TIME_LIMIT,
        memory_limit=MEMORY_LIMIT)

    # Parse the commandline and run the specified steps.
    exp.run_steps()


if __name__ == '__main__':
    main()
