#! /usr/bin/env python
import itertools
import os

from os import path
from helper import parse_json, layout_results, generate_latex_pagebreak, generate_latex_section, generate_latex_report


here = os.path.abspath(os.path.dirname(__file__))
ALGORITHMS = {'5ba85646-bfws-r0': 'BFWS(R_0)',
              '5ba85646-bfws-rall': 'BFWS(R_all)',
              '5ba85646-bfws-rg': 'BFWS(R_G)'}

means = ['iw1_reached_subgoals', 'iw2_reached_subgoals', 'num_reach_actions', 'num_state_vars', 'total_simulation_time',
         'iw_precondition_reachability']
aggrs = list(ALGORITHMS.values())


def collect_results_ipc18():
    results = dict()  # We'll leave all results here

    # Collect some data about IW simulations and problem stats alone
    data = parse_json(path.join(here, 'data/fs-tarski-prec_reachab_ipc18-eval/properties'))
    for _, run in data.items():
        copy_run_data(run, results)

    # Collect data about BFWS performance
    data = parse_json(path.join(here, 'data/fs-tarski-bfws_stats_ipc18-eval/properties'))
    # Discard results from buggy r_all configuration:
    data = {k: v for k, v in data.items() if '5ba85646-bfws-rall' not in k}

    data.update(parse_json(path.join(here, 'data/fs-tarski-bfws_stats_ipc18_rall-eval/properties')))

    for _, run in data.items():
        problem_id = (run['domain'], run['problem'])
        algo = ALGORITHMS[run['algorithm']]
        results[problem_id][algo] = int('coverage' in run and run['coverage'] == 1)

    return layout_results(results, means, aggrs)


def collect_results_jcai17():
    results = dict()  # We'll leave all results here

    # Collect some data about IW simulations and problem stats alone
    # data = parse_json(path.join(here, 'data/fs-tarski-iws_ijcai17-eval/properties'))
    data = parse_json(path.join(here, 'data/fs-tarski-prec_reachab_ijcai17-eval/properties'))
    for _, run in data.items():
        copy_run_data(run, results)

    data = parse_json(path.join(here, 'data/fs-tarski-bfws_stats_ijcai17_rall-eval/properties'))
    data.update(parse_json(path.join(here, 'data/fs-tarski-bfws_stats_ijcai17_r0-eval/properties')))
    data.update(parse_json(path.join(here, 'data/fs-tarski-bfws_stats_ijcai17_rg-eval/properties')))

    for _, run in data.items():
        problem_id = (run['domain'], run['problem'])
        algo = ALGORITHMS[run['algorithm']]
        results[problem_id][algo] = int('coverage' in run and run['coverage'] == 1)

    return layout_results(results, means, aggrs)


def copy_run_data(run, results):
    problem_id = (run['domain'], run['problem'])
    results[problem_id] = {
        'problem': run['problem'],
        'iw1_reached_subgoals': run['iw1_reached_subgoals'],
        'iw2_reached_subgoals': run['iw2_reached_subgoals'],
        'num_reach_actions': run['num_reach_actions'],
        'num_state_vars': run['num_state_vars'],
        'total_simulation_time': run['total_simulation_time'],
        'iw_precondition_reachability': run['iw_precondition_reachability'],
    }


def main():
    ijcai17_dom, ijcai17_inst = collect_results_jcai17()
    ipc18_dom, ipc18_inst = collect_results_ipc18()
    content = ''
    content += generate_latex_section("IJCAI 17 Benchmarks", ijcai17_dom.round(2).to_latex())
    content += generate_latex_pagebreak()

    content += generate_latex_section("IPC 18 Benchmarks", ipc18_dom.round(2).to_latex())
    content += generate_latex_pagebreak()

    for dom, instance in itertools.chain(ijcai17_inst, ipc18_inst):
        content += generate_latex_section(dom, instance.round(2).to_latex())
    generate_latex_report(content, 'bfws-analysis.tex')


if __name__ == '__main__':
    main()
