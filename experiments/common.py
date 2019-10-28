#! /usr/bin/env python


import os
from os import path

import fslab
from lab.environments import LocalEnvironment
from fslab.environments import UPFSlurmEnvironment

from downward.reports.absolute import AbsoluteReport
import common_setup

here = path.abspath(path.dirname(__file__))


def generate_environment(email, partition='short', memory_limit=8000):
    if common_setup.is_test_run():
        return LocalEnvironment(processes=2)
    else:
        return UPFSlurmEnvironment(
            partition=partition,
            email=email,
            export=["PATH", "DOWNWARD_BENCHMARKS", "FSBENCHMARKS", "LD_LIBRARY_PATH"],
            memory_per_cpu='{}M'.format(memory_limit),
        )


def generate_benchmark_suite(problems):
    if common_setup.is_test_run():
        return ['blocks:probBLOCKS-4-0.pddl', 'gripper:prob01.pddl']
    else:
        return problems


def get_fsplanner_binary():
    return path.abspath(os.path.join(here, '..', 'run.py'))


# Create custom report class with suitable info and error attributes.
class BaseReport(AbsoluteReport):
    INFO_ATTRIBUTES = ['time_limit', 'memory_limit']
    ERROR_ATTRIBUTES = [
        'domain', 'problem', 'algorithm', 'unexplained_errors', 'error', 'node']


DEFAULT_ATTRIBUTES = [
    'coverage', 'error', 'evaluations', 'plan', 'total_time', 'memory']


def add_standard_experiment_steps(exp, attributes=DEFAULT_ATTRIBUTES):
    # Add custom parser
    fsparser_path = os.path.abspath(os.path.join(os.path.dirname(fslab.__file__), 'fsparser.py'))
    exp.add_parser(fsparser_path)

    # Add step that writes experiment files to disk.
    exp.add_step('build', exp.build)

    # Add step that executes all runs.
    exp.add_step('start', exp.start_runs)

    # Add step that collects properties from run directories and
    # writes them to *-eval/properties.
    exp.add_fetcher(name='fetch')

    # Make a report.
    exp.add_report(
        BaseReport(attributes=attributes),
        outfile='report.html')


def add_experiment_run(exp, invocator, task, time_limit, memory_limit):
    run = exp.add_run()
    # Create symbolic links and aliases. This is optional. We could also use absolute paths in add_command().
    run.add_resource('domain', task.domain_file, symlink=True)
    run.add_resource('problem', task.problem_file, symlink=True)

    run.add_command('run-planner',
                    invocator(),
                    time_limit=time_limit,
                    memory_limit=memory_limit)

    # AbsoluteReport needs the following properties: 'domain', 'problem', 'algorithm', 'coverage'.
    run.set_property('domain', task.domain)
    run.set_property('problem', task.problem)
    run.set_property('algorithm', 'brfs-sdd')

    # BaseReport needs the following properties:  'time_limit', 'memory_limit'.
    run.set_property('time_limit', time_limit)
    run.set_property('memory_limit', memory_limit)

    # Every run has to have a unique id in the form of a list.
    # The algorithm name is only really needed when there are multiple algorithms.
    run.set_property('id', ['brfs-sdd', task.domain, task.problem])
