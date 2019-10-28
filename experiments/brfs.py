#! /usr/bin/env python


import os

from lab.experiment import Experiment
from downward import suites
from common import generate_environment, get_fsplanner_binary, add_standard_experiment_steps, add_experiment_run, \
    generate_benchmark_suite

TIME_LIMIT = 1800
MEMORY_LIMIT = 64000


SUITE = [
    'blocks',
    'organic-synthesis-opt18-strips',
    'pipesworld-tankage',
    'pipesworld-notankage',
]

BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]


def invocator():
    # We use Lab's data directory as workspace.
    return [get_fsplanner_binary(), '--domain', '{domain}', '-i', '{problem}', '--sdd', '--driver', 'bfs-sdd',
            '--output', '.']


def main():
    environment = generate_environment(email="guillem.frances@upf.edu",
                                       memory_limit=MEMORY_LIMIT, time_limit=TIME_LIMIT)
    exp = Experiment(environment=environment)

    suite = generate_benchmark_suite(SUITE)

    for task in suites.build_suite(BENCHMARKS_DIR, suite):
        add_experiment_run(exp, invocator=invocator, task=task, time_limit=TIME_LIMIT, memory_limit=MEMORY_LIMIT)

    add_standard_experiment_steps(exp)

    # Parse the commandline and run the specified steps.
    exp.run_steps()


if __name__ == '__main__':
    main()
