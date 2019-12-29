#! /usr/bin/env python
import os

from fslab.common_setup import FSIssueExperiment, IssueConfig
from common import generate_environment, filter_benchmarks_if_test_run, ALL_ATTRIBUTES, add_standard_experiment_steps

TIME_LIMIT = 1800
MEMORY_LIMIT = 8000

SUITE = [
    'agricola-sat18-strips',
    'caldera-sat18-adl',
    'caldera-split-sat18-adl',
    'data-network-sat18-strips',
    'flashfill-sat18-adl',
    'nurikabe-sat18-adl',
    'organic-synthesis-sat18-strips',
    'organic-synthesis-split-sat18-strips',
    'settlers-sat18-adl',
    'snake-sat18-strips',
    'spider-sat18-strips',
    'termes-sat18-strips',

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
REVISIONS = [
    "b6c63856",  # fs-sdd with old python frontend
    "f7830b46"   # The shiny new fs with the tarski frontend
]
BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]


def configs():
    ray = ['--asp', '--driver', 'sbfws', "--options", "verbose_stats=true,successor_generation=adaptive,evaluator_t=adaptive,bfws.rs=none"]
    sim = ['--asp', '--driver', 'sbfws', "--options", "verbose_stats=true,successor_generation=adaptive,evaluator_t=adaptive,bfws.rs=sim,sim.r_g_prime=true,width.simulation=2,sim.act_cutoff=40000"]
    return [
        IssueConfig("bfws-ray", ray),
        IssueConfig("bfws-sim", sim),
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
