#! /usr/bin/env python
import itertools
import os

from lab.experiment import Experiment
from downward import suites
from common import generate_environment, get_fsplanner_binary, add_all_runs, \
    filter_benchmarks_if_test_run, ALL_ATTRIBUTES

TIME_LIMIT = 1800
MEMORY_LIMIT = 8000

SUITE = [
    'organic-synthesis-split-sat18-strips',
]


BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]


def algorithms():
    # We use Lab's data directory as workspace.
    common = [get_fsplanner_binary(), '--domain', '{domain}', '-i', '{problem}', '--output', '.', '--asp']

    # With Simulation
    # ./run.py --asp -i ${PROBLEMFILE}  --driver sbfws --options "successor_generation=adaptive,evaluator_t=adaptive,bfws.rs=sim,sim.r_g_prime=true,width.simulation=2,sim.act_cutoff=40000"

    # We don't use the ASP reachability analysis at the moment to be as close as possible as the above experiment
    simu = common + ['--driver', 'sbfws', "--options", "verbose_stats=true,successor_generation=adaptive,evaluator_t=adaptive,bfws.rs=sim,sim.r_g_prime=true,width.simulation=2,sim.act_cutoff=40000"]

    return {
        'bfws-sim': simu
    }


def main():
    environment = generate_environment(email="guillem.frances@upf.edu",
                                       memory_limit=MEMORY_LIMIT, time_limit=TIME_LIMIT)
    exp = Experiment(environment=environment)

    benchmarks = suites.build_suite(os.environ["DOWNWARD_BENCHMARKS"], SUITE)
    benchmarks = filter_benchmarks_if_test_run(benchmarks)

    attributes = [a for a in ALL_ATTRIBUTES.keys() if not a.startswith('sdd')]

    add_all_runs(
        experiment=exp,
        suites=benchmarks,
        algorithms=algorithms(),
        time_limit=TIME_LIMIT,
        memory_limit=MEMORY_LIMIT,
        attributes=attributes,
        add_parse_step=True
    )

    # Parse the commandline and run the specified steps.
    exp.run_steps()


if __name__ == '__main__':
    main()
