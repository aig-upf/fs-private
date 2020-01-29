#! /usr/bin/env python
import json
from os import path
from statistics import mean

here = path.abspath(path.dirname(__file__))
ALGORITHMS = {'5ba85646-bfws-r0': 'BFWS(R_0)',
              '5ba85646-bfws-rall': 'BFWS(R_all)',
              '5ba85646-bfws-rg': 'BFWS(R_G)'}


def parse_json(filename):
    with open(filename, 'r') as f:
        data = json.load(f)
    return data


def print_results(results, name):
    import pandas as pd

    means = ['iw1_reached_subgoals', 'iw2_reached_subgoals', 'num_reach_actions', 'num_state_vars', 'total_simulation_time']
    aggrs = list(ALGORITHMS.values())

    domains = sorted(set(k[0] for k in results.keys()))
    per_domain = []

    with open(path.join(here, f'instances_{name}.txt'), 'w') as f:
        for d in domains:
            instances = [v for k, v in results.items() if k[0] == d]
            summary = {'problem': d}
            for a in means:
                summary[a] = mean(v[a] for v in instances)
            for a in aggrs:
                summary[a] = sum(v[a] for v in instances)
            per_domain.append(summary)

            print(f'\n\n{d}\n{pd.DataFrame(instances)}')
            print(f'\n\n{d}\n{pd.DataFrame(instances)}', file=f)

    total_summary = {'problem': 'Summary'}
    for a in means:
        total_summary[a] = mean(v[a] for v in per_domain)
    for a in aggrs:
        total_summary[a] = sum(v[a] for v in per_domain)

    with open(path.join(here, f'domains_{name}.txt'), 'w') as f:
        print(f"\n\nAverages\n{pd.DataFrame(per_domain + [total_summary]).round(2)}")
        print(f"\n\nAverages\n{pd.DataFrame(per_domain + [total_summary]).round(2)}", file=f)


def main():
    collect_results_jcai17()
    collect_results_ipc18()


def collect_results_ipc18():
    results = dict()  # We'll leave all results here

    # Collect some data about IW simulations and problem stats alone
    data = parse_json(path.join(here, 'data/fs-tarski-iws_ipc18-eval/properties'))
    for _, run in data.items():
        if run['algorithm'] != '1d8325d7-bfws-iw2-mt_reach':  # We'll take the data from the run with reachability
            continue
        problem_id = (run['domain'], run['problem'])
        results[problem_id] = {
            'problem': run['problem'],
            'iw1_reached_subgoals': run['iw1_reached_subgoals'],
            'iw2_reached_subgoals': run['iw2_reached_subgoals'],
            'num_reach_actions': run['num_reach_actions'],
            'num_state_vars': run['num_state_vars'],
            'total_simulation_time': run['total_simulation_time'],
        }

    # Collect data about BFWS performance
    data = parse_json(path.join(here, 'data/fs-tarski-bfws_stats_ipc18-eval/properties'))
    # Discard results from buggy r_all configuration:
    data = {k: v for k, v in data.items() if '5ba85646-bfws-rall' not in k}

    data.update(parse_json(path.join(here, 'data/fs-tarski-bfws_stats_ipc18_rall-eval/properties')))

    for _, run in data.items():
        problem_id = (run['domain'], run['problem'])
        algo = ALGORITHMS[run['algorithm']]
        results[problem_id][algo] = int('coverage' in run and run['coverage'] == 1)

    print_results(results, "ipc18")


def collect_results_jcai17():
    results = dict()  # We'll leave all results here

    # Collect some data about IW simulations and problem stats alone
    data = parse_json(path.join(here, 'data/fs-tarski-iws_ijcai17-eval/properties'))
    for _, run in data.items():
        problem_id = (run['domain'], run['problem'])
        results[problem_id] = {
            'problem': run['problem'],
            'iw1_reached_subgoals': run['iw1_reached_subgoals'],
            'iw2_reached_subgoals': run['iw2_reached_subgoals'],
            'num_reach_actions': run['num_reach_actions'],
            'num_state_vars': run['num_state_vars'],
            'total_simulation_time': run['total_simulation_time'],
        }

    data = parse_json(path.join(here, 'data/fs-tarski-bfws_stats_ijcai17_rall-eval/properties'))
    data.update(parse_json(path.join(here, 'data/fs-tarski-bfws_stats_ijcai17_r0-eval/properties')))
    data.update(parse_json(path.join(here, 'data/fs-tarski-bfws_stats_ijcai17_rg-eval/properties')))

    for _, run in data.items():
        problem_id = (run['domain'], run['problem'])
        algo = ALGORITHMS[run['algorithm']]
        results[problem_id][algo] = int('coverage' in run and run['coverage'] == 1)

    print_results(results, "ijcai17")


if __name__ == '__main__':
    main()
