#! /usr/bin/env python
import itertools
import os
from os import path

import fslab
from lab.environments import LocalEnvironment
from fslab.environments import UPFSlurmEnvironment

from downward import suites
from downward.reports.absolute import AbsoluteReport
import common_setup
from lab.reports import Attribute

here = path.abspath(path.dirname(__file__))


def generate_environment(email, partition='short', time_limit=1800, memory_limit=8000):
    if common_setup.is_test_run():
        return LocalEnvironment(processes=2)
    else:
        return UPFSlurmEnvironment(
            partition=partition,
            email=email,
            export=["PATH", "DOWNWARD_BENCHMARKS", "FSBENCHMARKS", "LD_LIBRARY_PATH", "GRINGO_PATH"],
            time_limit=round(time_limit/60),
            memory_per_cpu='{}M'.format(memory_limit),
        )


def filter_benchmarks_if_test_run(problems):
    if common_setup.is_test_run():
        return suites.build_suite(os.environ["DOWNWARD_BENCHMARKS"], ['blocks:probBLOCKS-4-0.pddl', 'gripper:prob01.pddl'])
    else:
        return problems


def get_fsplanner_binary():
    return path.abspath(os.path.join(here, '..', 'run.py'))


# Create custom report class with suitable info and error attributes.
class BaseReport(AbsoluteReport):
    INFO_ATTRIBUTES = ['time_limit', 'memory_limit']
    ERROR_ATTRIBUTES = [
        'domain', 'problem', 'algorithm', 'unexplained_errors', 'error', 'node']


ALL_ATTRIBUTES = {
    'coverage': 'coverage',
    'run_dir': 'run_dir',
    'error': 'error',
    'generations': 'generations',
    'plan': 'plan',
    'total_time': 'total_time',
    'memory': 'memory',
    'node_generation_rate': Attribute('node_generation_rate', min_wins=False),
    'sdd_sizes': Attribute('sdd_sizes', min_wins=True),
    'sdd_theory_vars': Attribute('sdd_theory_vars', min_wins=True),
    'sdd_theory_constraints': Attribute('sdd_theory_constraints', min_wins=True),
    'successor_generator': 'successor_generator',
    'num_reach_actions': 'num_reach_actions',
    'asp_prep_time': 'asp_prep_time',
    'asp_prep_mem': 'asp_prep_mem',
    'mem_before_mt': 'mem_before_mt',
    'mem_before_search': 'mem_before_search',
    'last_recorded_time': 'last_recorded_time',
    'last_recorded_generations': Attribute('last_recorded_generations', min_wins=True),
    'num_state_vars': 'num_state_vars',
}

DEFAULT_ATTRIBUTES = [
    "coverage",
    "error",
    "generated",
    "memory",
    "run_dir",
    "total_time",
]


def add_standard_experiment_steps(exp, attributes=None, add_parse_step=False):

    attributes = attributes or DEFAULT_ATTRIBUTES
    attributes = [ALL_ATTRIBUTES[x] for x in attributes]

    # Add custom parser
    fsparser_path = os.path.abspath(os.path.join(os.path.dirname(fslab.__file__), 'fsparser.py'))
    exp.add_parser(fsparser_path)

    # Add step that writes experiment files to disk.
    exp.add_step('build', exp.build)

    # Add step that executes all runs.
    exp.add_step('start', exp.start_runs)

    if add_parse_step:
        exp.add_parse_again_step()

    # Add step that collects properties from run directories and
    # writes them to *-eval/properties.
    exp.add_fetcher(name='fetch')

    # Make a report.
    exp.add_report(
        BaseReport(attributes=attributes),
        outfile='report.html')


def add_all_runs(experiment, suites, algorithms, time_limit, memory_limit, attributes=None, add_parse_step=False):
    add_standard_experiment_steps(experiment, attributes, add_parse_step)

    for task, algo in itertools.product(suites, algorithms.keys()):
        add_experiment_run(algorithm=algo, exp=experiment,
                           invocator=lambda: algorithms[algo],
                           task=task, time_limit=time_limit, memory_limit=memory_limit)


def add_experiment_run(algorithm, exp, invocator, task, time_limit, memory_limit):
    run = exp.add_run()
    # Create symbolic links and aliases. This is optional. We could also use absolute paths in add_command().
    run.add_resource('domain', task.domain_file, symlink=True)
    run.add_resource('problem', task.problem_file, symlink=True)

    run.add_command('run-planner',
                    invocator(),
                    time_limit=time_limit,
                    memory_limit=memory_limit,
                    soft_stdout_limit=None, hard_stdout_limit=None,
                    soft_stderr_limit=None, hard_stderr_limit=None,)

    # AbsoluteReport needs the following properties: 'domain', 'problem', 'algorithm', 'coverage'.
    run.set_property('domain', task.domain)
    run.set_property('problem', task.problem)
    run.set_property('algorithm', algorithm)

    # BaseReport needs the following properties:  'time_limit', 'memory_limit'.
    run.set_property('time_limit', time_limit)
    run.set_property('memory_limit', memory_limit)

    # Every run has to have a unique id in the form of a list.
    # The algorithm name is only really needed when there are multiple algorithms.
    run.set_property('id', [algorithm, task.domain, task.problem])
