#! /usr/bin/env python
import itertools
import json
from os import path
import subprocess
from statistics import mean

import pandas as pd

pd.set_option('display.max_colwidth', -1)
pd.set_option('display.max_columns', None)

here = path.abspath(path.dirname(__file__))
ALGORITHMS = {'5ba85646-bfws-r0': 'BFWS(R_0)',
              '5ba85646-bfws-rall': 'BFWS(R_all)',
              '5ba85646-bfws-rg': 'BFWS(R_G)'}


def parse_json(filename):
    with open(filename, 'r') as f:
        data = json.load(f)
    return data


def print_results(results, name):
    means = ['iw1_reached_subgoals', 'iw2_reached_subgoals', 'num_reach_actions', 'num_state_vars', 'total_simulation_time', 'iw_precondition_reachability']
    aggrs = list(ALGORITHMS.values())

    domains = sorted(set(k[0] for k in results.keys()))
    per_domain = []
    per_instance = []

    with open(path.join(here, f'instances_{name}.txt'), 'w') as f:
        for d in domains:
            instances = [v for k, v in results.items() if k[0] == d]
            summary = {'problem': d}
            for a in means:
                summary[a] = mean(v[a] for v in instances)
            for a in aggrs:
                summary[a] = sum(v[a] for v in instances)
            per_domain.append(summary)

            # print(f'\n\n{d}\n{pd.DataFrame(instances)}')
            # print(f'\n\n{d}\n{pd.DataFrame(instances)}', file=f)
            per_instance.append((d, pd.DataFrame(instances)))

    total_summary = {'problem': 'Summary'}
    for a in means:
        total_summary[a] = mean(v[a] for v in per_domain)
    for a in aggrs:
        total_summary[a] = sum(v[a] for v in per_domain)

    return pd.DataFrame(per_domain + [total_summary]), per_instance


def generate_latex_report(content, filename):
    with open(path.join(here, 'tpl.tex'), 'r') as f:
        tpl = f.read()
    output = path.join(here, filename)
    with open(output, 'w') as f:
        print(tpl.replace('#Content#', content), file=f)
    subprocess.run(["ls", "-l"])
    subprocess.run(f"pdflatex {filename}".split(),  check=True)


def generate_latex_section(title, content):
    return f'\\section{{{title}}}\n\n{content}'


def generate_latex_pagebreak():
    return f'\\newpage'


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

    return print_results(results, "ipc18")


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

    return print_results(results, "ijcai17")


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
