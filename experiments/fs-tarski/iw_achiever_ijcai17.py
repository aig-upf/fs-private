#! /usr/bin/env python
import os

from fslab.common_setup import FSIssueExperiment, IssueConfig
from common import generate_environment, filter_benchmarks_if_test_run, ALL_ATTRIBUTES, add_standard_experiment_steps

TIME_LIMIT = 1800
MEMORY_LIMIT = 8000


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
    'parcprinter-sat11-strips',
    # 'parking-sat11-strips',
    'parking-sat14-strips',
    'pegsol-sat11-strips',
    'scanalyzer-sat11-strips',
    'sokoban-sat11-strips',
    'tetris-sat14-strips',
    'thoughtful-sat14-strips',
    'tidybot-sat11-strips',
    # 'transport-sat11-strips',
    'transport-sat14-strips',
    # 'visitall-sat11-strips',
    'visitall-sat14-strips',
    'woodworking-sat11-strips',
]

REVISIONS = [
    "9a10f5fb",
]
BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]


def configs():
    grounded_mt_reach_rg = '--driver sbfws --options "verbose_stats=true,successor_generation=match_tree,evaluator_t=adaptive,bfws.rs=sim,sim.r_g_prime=true,width.simulation=2"'  # i.e. no action cutoff
    grounded_mt_reach_ach = '--driver sbfws --options "verbose_stats=true,successor_generation=match_tree,evaluator_t=adaptive,bfws.rs=sim,sim.r_g_prime=true,width.simulation=1,sim.achiever_novelty=true"'  # i.e. no action cutoff

    return [
        IssueConfig("bfws-rg-std", grounded_mt_reach_rg.split()),
        IssueConfig("bfws-rg-ach", grounded_mt_reach_ach.split()),
    ]


def main():
    environment = generate_environment(email="guillem.frances@upf.edu",
                                       memory_limit=MEMORY_LIMIT, time_limit=TIME_LIMIT)
    exp = FSIssueExperiment(
        revisions=REVISIONS,
        configs=configs(),
        environment=environment,
        time_limit=TIME_LIMIT,
        memory_limit=MEMORY_LIMIT
    )

    benchmarks = filter_benchmarks_if_test_run(SUITE)

    attributes = [att for name, att in ALL_ATTRIBUTES.items() if not name.startswith('sdd')]

    exp.add_suite(os.environ["DOWNWARD_BENCHMARKS"], benchmarks)

    add_standard_experiment_steps(exp, add_parse_step=True)

    exp.add_absolute_report_step(attributes=attributes)
    # exp.add_comparison_table_step()

    exp.run_steps()  # Let's go!


if __name__ == '__main__':
    main()
