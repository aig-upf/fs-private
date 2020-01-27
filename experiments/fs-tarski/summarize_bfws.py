#! /usr/bin/env python
import json
from os import path

here = path.abspath(path.dirname(__file__))


def main():
    iws_ipc = path.join(here, 'data/fs-tarski-iws_ipc20-eval/properties')

    results = dict()  # We'll leave all results here
    with open(iws_ipc, 'r') as f:
        data = json.load(f)

    # Collect some data about IW simulations and problem stats alone
    for _, run in data.items():
        if run['algorithm'] != '1d8325d7-bfws-iw2-mt_each':  # We'll take the data from the run with reachability
            continue
        problem_id = (run['domain'], run['problem'])
        results[problem_id] = {
            'iw1_reached_subgoals': run['iw1_reached_subgoals'],
            'iw2_reached_subgoals': run['iw2_reached_subgoals'],
            'num_reach_actions': run['num_reach_actions'],
            'num_state_vars': run['num_state_vars'],
            'total_simulation_time': run['total_simulation_time'],
        }

    print('Done!')


if __name__ == '__main__':
    main()
