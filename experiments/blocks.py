#! /usr/bin/env python


import os
import platform
from os import path

from lab.environments import LocalEnvironment
import fslab
from fslab.environments import UPFSlurmEnvironment
from lab.experiment import Experiment

from downward import suites
from downward.reports.absolute import AbsoluteReport

import common_setup
from common_setup import IssueExperiment

here = path.abspath(path.dirname(__file__))


# Create custom report class with suitable info and error attributes.
class BaseReport(AbsoluteReport):
    INFO_ATTRIBUTES = ['time_limit', 'memory_limit']
    ERROR_ATTRIBUTES = [
        'domain', 'problem', 'algorithm', 'unexplained_errors', 'error', 'node']


SUITE = [
    'blocks:probBLOCKS-4-0.pddl',
    'blocks:probBLOCKS-4-1.pddl',
    'blocks:probBLOCKS-4-2.pddl',
]

is_local = not platform.node().startswith("node")
BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]

ENV = UPFSlurmEnvironment(
    partition='short',
    email="guillem.frances@upf.edu",
    export=["PATH", "DOWNWARD_BENCHMARKS", "FSBENCHMARKS"]
)

if common_setup.is_test_run():
    SUITE = IssueExperiment.DEFAULT_TEST_SUITE
    ENV = LocalEnvironment(processes=2)

exp = Experiment(environment=ENV)


ATTRIBUTES = [
    'coverage', 'error', 'evaluations', 'plan', 'total_time', 'memory']
TIME_LIMIT = 1800
MEMORY_LIMIT = 8000

PLANNER_BINARY = path.abspath(os.path.join(here, '..', 'run.py'))
# ./run.py --sdd -i /home/frances/projects/code/downward-benchmarks/blocks/probBLOCKS-4-0.pddl --driver bfs-sdd

# Use Lab's data directory as workspace.
PLANNER_INVOCATION = [PLANNER_BINARY, '--domain', '{domain}', '-i', '{problem}', '--sdd', '--driver', 'bfs-sdd',
                      '--output', '.']


# Add custom parser
fsparser_path = os.path.abspath(os.path.join(os.path.dirname(fslab.__file__), 'fsparser.py'))
exp.add_parser(fsparser_path)

for task in suites.build_suite(BENCHMARKS_DIR, SUITE):
    run = exp.add_run()
    # Create symbolic links and aliases. This is optional. We could also use absolute paths in add_command().
    run.add_resource('domain', task.domain_file, symlink=True)
    run.add_resource('problem', task.problem_file, symlink=True)

    run.add_command(
        'run-planner',
        PLANNER_INVOCATION,
        time_limit=TIME_LIMIT,
        memory_limit=MEMORY_LIMIT)

    # AbsoluteReport needs the following properties: 'domain', 'problem', 'algorithm', 'coverage'.
    run.set_property('domain', task.domain)
    run.set_property('problem', task.problem)
    run.set_property('algorithm', 'brfs-sdd')

    # BaseReport needs the following properties:  'time_limit', 'memory_limit'.
    run.set_property('time_limit', TIME_LIMIT)
    run.set_property('memory_limit', MEMORY_LIMIT)

    # Every run has to have a unique id in the form of a list.
    # The algorithm name is only really needed when there are multiple algorithms.
    run.set_property('id', ['brfs-sdd', task.domain, task.problem])

# Add step that writes experiment files to disk.
exp.add_step('build', exp.build)

# Add step that executes all runs.
exp.add_step('start', exp.start_runs)

# Add step that collects properties from run directories and
# writes them to *-eval/properties.
exp.add_fetcher(name='fetch')

# Make a report.
exp.add_report(
    BaseReport(attributes=ATTRIBUTES),
    outfile='report.html')

# Parse the commandline and run the specified steps.
exp.run_steps()
