#! /usr/bin/env python
from lab.experiment import Experiment
from common import ALL_ATTRIBUTES, BaseReport


def main():

    # Use the path where the combined results will be left:
    exp = Experiment('/homedtic/gfrances/projects/code/fs-sdd/experiments/data/combined')

    # Use the paths where the results of the different experiments to be combined are:
    exp.add_fetcher('/homedtic/gfrances/projects/code/fs-sdd/experiments/fs-tarski/data/fs-tarski-sat-eval')
    exp.add_fetcher('/homedtic/gfrances/projects/code/fs-sdd/experiments/fs-tarski/data/fs-tarski-sat_r-eval')

    # Use the filter_algorithm parameter if you only need some algos in the table, e.g.
    # algos=['brfs-naive', 'brfs-sdd-asp', 'brfs-sdd-vo-asp']
    # exp.add_fetcher('/homedtic/gfrances/projects/code/fs-sdd/experiments/fs-tarski/data/fs-tarski-sat-eval', filter_algorithm=algos)

    attributes = [att for name, att in ALL_ATTRIBUTES.items() if not name.startswith('sdd')]
    exp.add_report(
        BaseReport(attributes=attributes),
        outfile='report.html')

    # Parse the commandline and run the specified steps.
    exp.run_steps()


if __name__ == '__main__':
    main()
