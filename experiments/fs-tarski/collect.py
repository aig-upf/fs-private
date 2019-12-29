#! /usr/bin/env python
from lab.experiment import Experiment
from common import ALL_ATTRIBUTES, BaseReport


def main():

    algos=['brfs-naive', 'brfs-sdd-asp', 'brfs-sdd-vo-asp']
    algos=None

    exp = Experiment('/homedtic/gfrances/projects/code/fs-sdd/experiments/data/combined')
    exp.add_fetcher('/homedtic/gfrances/projects/code/fs-sdd/experiments/fs-tarski/data/fs-tarski-sat-eval', filter_algorithm=algos)
    exp.add_fetcher('/homedtic/gfrances/projects/code/fs-sdd/experiments/fs-tarski/data/fs-tarski-sat_r-eval', filter_algorithm=algos)

    attributes = [att for name, att in ALL_ATTRIBUTES.items() if not name.startswith('sdd')]
    exp.add_report(
        BaseReport(attributes=attributes),
        outfile='report.html')

    # Parse the commandline and run the specified steps.
    exp.run_steps()


if __name__ == '__main__':
    main()
