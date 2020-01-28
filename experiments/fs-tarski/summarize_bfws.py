#! /usr/bin/env python
import json
from os import path
from statistics import mean

here = path.abspath(path.dirname(__file__))
ALGORITHMS = {'5ba85646-bfws-r0': 'BFWS(R_0)',
              '5ba85646-bfws-rall': 'BFWS(R_all)',
              '5ba85646-bfws-rg': 'BFWS(R_G)'}


def print_results(results):
    import pandas as pd

    means = ['iw1_reached_subgoals', 'iw2_reached_subgoals', 'num_reach_actions', 'num_state_vars']
    aggrs = list(ALGORITHMS.values())

    domains = sorted(set(k[0] for k in results.keys()))
    per_domain = []

    for d in domains:
        instances = [v for k, v in results.items() if k[0] == d]
        summary = {'problem': d}
        for a in means:
            summary[a] = mean(v[a] for v in instances)
        for a in aggrs:
            summary[a] = sum(v[a] for v in instances)
        per_domain.append(summary)
        print(f'\n\n{d}')
        print(pd.DataFrame(instances))
        # print(pd.DataFrame(instances + [summary]))

    total_summary = {'problem': 'Summary'}
    for a in means:
        total_summary[a] = mean(v[a] for v in per_domain)
    for a in aggrs:
        total_summary[a] = sum(v[a] for v in per_domain)

    print("\n\nAverages")
    print(pd.DataFrame(per_domain + [total_summary]).round(2))


def main():
    results = dict()  # We'll leave all results here

    # Collect some data about IW simulations and problem stats alone
    with open(path.join(here, 'data/fs-tarski-iws_ipc20-eval/properties'), 'r') as f:
        data = json.load(f)

    for _, run in data.items():
        if run['algorithm'] != '1d8325d7-bfws-iw2-mt_each':  # We'll take the data from the run with reachability
            continue
        problem_id = (run['domain'], run['problem'])
        results[problem_id] = {
            'problem': run['problem'],
            'iw1_reached_subgoals': run['iw1_reached_subgoals'],
            'iw2_reached_subgoals': run['iw2_reached_subgoals'],
            'num_reach_actions': run['num_reach_actions'],
            'num_state_vars': run['num_state_vars'],
            # 'total_simulation_time': run['total_simulation_time'],
        }

    # Collect data about BFWS performance
    with open(path.join(here, 'data/fs-tarski-bfws_stats_ipc20-eval/properties'), 'r') as f:
        data = json.load(f)

    for _, run in data.items():
        problem_id = (run['domain'], run['problem'])
        algo = ALGORITHMS[run['algorithm']]
        results[problem_id][algo] = int('coverage' in run and run['coverage'] == 1)

    print_results(results)
    print('Done!')


if __name__ == '__main__':
    main()
