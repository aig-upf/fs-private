#! /usr/bin/env python
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
    'floortile-opt11-strips',
    'hiking-opt14-strips',
    'organic-synthesis-opt18-strips',
]

ORGSYNTH_SUITE = [
    'organic-synthesis',
]


def algorithms():
    # We use Lab's data directory as workspace.
    base = [get_fsplanner_binary(), '--domain', '{domain}', '-i', '{problem}', '--output', '.']
    lifted = base + ['--sdd', '--driver', 'sbfws-sdd', '--options', '"sdd.minimization_time=100,evaluator_t=adaptive,bfws.rs=sim,sim.r_all=true"',
                     "--var_ordering", "arity", "--sdd_with_reachability"]
    grounded = base + ['--driver', 'sbfws',
                       '--options', '"successor_generation=adaptive,evaluator_t=adaptive,bfws.rs=sim,sim.r_all=true"']

    # --options="verbose_stats=true,sdd.minimization_time=100,evaluator_t=adaptive,bfws.rs=sim,sim.r_all=true" --var_ordering=arity


    return {'bfws-r_all-sdd-asp-vo': lifted, 'bfws-r_all-ground': grounded}


def main():
    environment = generate_environment(email="guillem.frances@upf.edu", memory_limit=MEMORY_LIMIT)
    exp = Experiment(environment=environment)

    benchmarks = suites.build_suite(os.environ["DOWNWARD_BENCHMARKS"], SUITE)
    #
    # benchmarks += suites.build_suite(
    #     os.path.join(os.environ["HOME"], "projects", "code", "organic-synthesis-benchmarks"),
    #     ORGSYNTH_SUITE)

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
