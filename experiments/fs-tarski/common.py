#! /usr/bin/env python
import os
from os import path
from statistics import mean

import fslab
from lab.environments import LocalEnvironment
from fslab.environments import UPFSlurmEnvironment
from fslab import common_setup

from downward.reports.absolute import AbsoluteReport
from lab.reports import Attribute

here = path.abspath(path.dirname(__file__))


def generate_environment(email, partition='short', time_limit=1800, memory_limit=8000):
    if common_setup.is_test_run():
        return LocalEnvironment(processes=2)
    else:
        return UPFSlurmEnvironment(
            partition=partition,
            email=email,
            # The GRINGO_PATH variable was necessary for the older, non-tarski versions of the planner
            export=["PATH", "DOWNWARD_BENCHMARKS", "FSBENCHMARKS", "LIBRARY_PATH", "LD_LIBRARY_PATH", "GRINGO_PATH"],
            time_limit=round(time_limit/60),
            memory_per_cpu='{}M'.format(memory_limit),
        )


def filter_benchmarks_if_test_run(problems):
    return ['blocks:probBLOCKS-4-0.pddl', 'gripper:prob01.pddl'] if common_setup.is_test_run() else problems


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

    'num_state_vars': 'num_state_vars',
    'num_action_schemas': 'num_action_schemas',
    'num_ground_actions': 'num_ground_actions',
    'num_reach_actions': 'num_reach_actions',
    'successor_generator': 'successor_generator',

    'mem_before_mt': 'mem_before_mt',
    'mem_before_search': 'mem_before_search',
    'node_generation_rate': Attribute('node_generation_rate', function=mean, min_wins=False),
    'last_recorded_generations': Attribute('last_recorded_generations', min_wins=True),

    'time_backend': 'time_backend',
    'time_frontend': 'time_frontend',
    'last_recorded_time': 'last_recorded_time',

    # ASP reachability analysis
    'reach_time': Attribute('reach_time', function=sum, min_wins=True),
    'reach_mem': Attribute('reach_mem', function=sum, min_wins=True),

    # IW simulation attributes
    'sim_iw1_started': Attribute('sim_iw1_started', function=sum, min_wins=False),
    'sim_iw2_started': Attribute('sim_iw2_started', function=sum, min_wins=False),
    'sim_iw1_reached_subgoals': Attribute('iw1_reached_subgoals', function=mean, min_wins=False),
    'sim_iw2_reached_subgoals': Attribute('iw2_reached_subgoals', function=mean, min_wins=False),
    'sim_total_simulation_time': Attribute('total_simulation_time', function=mean, min_wins=True),
    'sim_iw_precondition_reachability': Attribute('iw_precondition_reachability', function=mean),
    'sim_iw1_successful': Attribute('sim_iw1_successful', min_wins=False),
    'sim_rall_because_too_many_actions': Attribute('sim_rall_because_too_many_actions', min_wins=False),
    'sim_iw2_successful': Attribute('sim_iw2_successful', min_wins=False),
    'sim_successful': Attribute('sim_successful', min_wins=False),
    'sim_rall_because_iw2_unsuccessful': Attribute('sim_rall_because_iw2_unsuccessful', min_wins=False),
    'sim_goal_reached': 'sim_goal_reached',
    'sim_nodes_expanded': 'sim_nodes_expanded',
    'sim_iw1_finished': 'iw1_finished',
    'sim_iw2_finished': 'iw2_finished',

    # SDD attributes
    'sdd_sizes': Attribute('sdd_sizes', min_wins=True),
    'sdd_theory_vars': Attribute('sdd_theory_vars', min_wins=True),
    'sdd_theory_constraints': Attribute('sdd_theory_constraints', min_wins=True),
}


def add_standard_experiment_steps(exp, add_parse_step=False):
    # Add custom parser
    fsparser_path = os.path.abspath(os.path.join(os.path.dirname(fslab.__file__), 'fsparser.py'))
    exp.add_parser(fsparser_path)

    # Add step that writes experiment files to disk.
    exp.add_step('build', exp.build)

    # Add step that executes all runs.
    exp.add_step('start', exp.start_runs)

    if add_parse_step:
        exp.add_parse_again_step()

    # Add step that collects properties from run directories and writes them to *-eval/properties.
    exp.add_fetcher(name='fetch')
