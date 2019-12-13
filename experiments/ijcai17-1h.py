#! /usr/bin/env python
import itertools
import os

from lab.experiment import Experiment
from downward import suites
from common import generate_environment, get_fsplanner_binary, add_all_runs, \
    filter_benchmarks_if_test_run, ALL_ATTRIBUTES

TIME_LIMIT = 3600
MEMORY_LIMIT = 16000

# Benchmarked problems include all instances from the last
# planning competition (IPC 2014), along with all instances
# from IPC 2011 domains that did not appear in IPC 2014,
# with the exception of Parcprinter, Tidybot and Woodworking,
# which produced parsing errors. There are thus a total
# of 19 domains, with 20 instances each, for a total of 380 instances
SUITE = [
    # 'barman-sat11-strips',
    'barman-sat14-strips',
    'cavediving-14-adl',
    'childsnack-sat14-strips',
    'citycar-sat14-adl',
    'elevators-sat11-strips',
    # 'floortile-sat11-strips',
    'floortile-sat14-strips',
    'ged-sat14-strips',
    'hiking-sat14-strips',
    'maintenance-sat14-adl',
    'nomystery-sat11-strips',
    # 'openstacks-sat11-strips',
    'openstacks-sat14-strips',
    # 'parcprinter-sat11-strips',
    # 'parking-sat11-strips',
    'parking-sat14-strips',
    'pegsol-sat11-strips',
    'scanalyzer-sat11-strips',
    'sokoban-sat11-strips',
    'tetris-sat14-strips',
    'thoughtful-sat14-strips',
    # 'tidybot-sat11-strips',
    # 'transport-sat11-strips',
    'transport-sat14-strips',
    # 'visitall-sat11-strips',
    'visitall-sat14-strips',
    # 'woodworking-sat11-strips',
]


BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]


def algorithms():
    # We use Lab's data directory as workspace.
    common = [get_fsplanner_binary(), '--domain', '{domain}', '-i', '{problem}', '--output', '.', '--asp']

    # Blind
    # ./run.py --asp -i /home/frances/projects/code/downward-benchmarks/agricola-sat18-strips/p01.pddl --driver sbfws --options="successor_generation=adaptive,evaluator_t=adaptive,bfws.rs=none"
    raycharles = common + ['--driver', 'sbfws', "--options", "verbose_stats=true,successor_generation=adaptive,evaluator_t=adaptive,bfws.rs=none"]

    # With Simulation
    # ./run.py --asp -i ${PROBLEMFILE}  --driver sbfws --options "successor_generation=adaptive,evaluator_t=adaptive,bfws.rs=sim,sim.r_g_prime=true,width.simulation=2,sim.act_cutoff=40000"

    # We don't use the ASP reachability analysis at the moment to be as close as possible as the above experiment
    simu = common + ['--driver', 'sbfws', "--options", "verbose_stats=true,successor_generation=adaptive,evaluator_t=adaptive,bfws.rs=sim,sim.r_g_prime=true,width.simulation=2,sim.act_cutoff=40000"]

    return {
        'bfws-ray': raycharles,
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
