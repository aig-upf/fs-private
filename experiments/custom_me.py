#! /usr/bin/env python
import itertools
import os

from lab.experiment import Experiment
from downward import suites
from common import generate_environment, get_fsplanner_binary, add_all_runs, \
    filter_benchmarks_if_test_run

TIME_LIMIT = 3600
MEMORY_LIMIT = 32000

SUITE = [
    'organic-synthesis-opt18-strips:p01.pddl',
    'organic-synthesis-opt18-strips:p02.pddl',
    # 'organic-synthesis-opt18-strips:p03.pddl',
    # 'organic-synthesis-opt18-strips:p04.pddl',
    'blocks',
    # 'blocks:probBLOCKS-9-0.pddl',
    # 'blocks:probBLOCKS-10-0.pddl',
    'pipesworld-tankage',
    # 'pipesworld-tankage:p02-net1-b6-g4-t50.pddl',
    # 'pipesworld-tankage:p03-net1-b8-g3-t80.pddl',
]

BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]


def algorithms():
    # We use Lab's data directory as workspace.
    lifted = [get_fsplanner_binary(), '--debug', '--domain', '{domain}', '-i', '{problem}', '--sdd', '--driver', 'bfs-sdd',
              '--output', '.']

    # return {'brfs-deb-sdd': lifted}
    grounded = [get_fsplanner_binary(), '--debug', '--domain', '{domain}', '-i', '{problem}', '--sdd', '--driver', 'bfs',
                '--output', '.']

    configs = dict()

    # for min_time, custom_me in itertools.product(['0', '100'], [True, False]):
    #     name = f'brfs-sdd-{min_time}' + ('-custom_me' if custom_me else '')
    #     configs[name] = lifted + ['--options']

    lifted += ['--options', '"sdd.minimization_time=100,sdd.custom_me=true"']

    return {'brfs-sdd': lifted, 'brfs-ground': grounded}


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
