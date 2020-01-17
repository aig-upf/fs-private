#! /usr/bin/env python
import os

from fslab.common_setup import FSIssueExperiment, IssueConfig
from common import generate_environment, filter_benchmarks_if_test_run, ALL_ATTRIBUTES, add_standard_experiment_steps

TIME_LIMIT = 1800
MEMORY_LIMIT = 8000

SUITE = [
    'agricola-sat18-strips',
    'caldera-sat18-adl',
    # 'caldera-split-sat18-adl',
    'data-network-sat18-strips',
    # 'flashfill-sat18-adl',
    'nurikabe-sat18-adl',
    'organic-synthesis-sat18-strips',
    'organic-synthesis-split-sat18-strips',
    # 'settlers-sat18-adl',
    'snake-sat18-strips',
    'spider-sat18-strips',
    'termes-sat18-strips',
]
REVISIONS = [
    "1d8325d7"   # Run IW simulations only
]
BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]


def configs():
    grounded_mt_noreach = ['--no-reachability', '--driver', 'sbfws', '--options', "verbose_stats=true,successor_generation=match_tree,evaluator_t=adaptive,bfws.rs=sim,sim.r_g_prime=true,width.simulation=2,sim.act_cutoff=40000"]
    grounded_mt_reach = ['--driver', 'sbfws', '--options', "verbose_stats=true,successor_generation=match_tree,evaluator_t=adaptive,bfws.rs=sim,sim.r_g_prime=true,width.simulation=2,sim.act_cutoff=40000"]

    return [
        IssueConfig("bfws-iw2-mt_noreach", grounded_mt_noreach),
        IssueConfig("bfws-iw2-mt_each", grounded_mt_reach),
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
