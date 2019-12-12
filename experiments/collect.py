#! /usr/bin/env python
from lab.experiment import Experiment
from common import ALL_ATTRIBUTES, BaseReport


def main():

    algos=['brfs-naive', 'brfs-sdd-asp', 'brfs-sdd-vo-asp']

    exp = Experiment('/homedtic/gfrances/projects/code/fs-sdd/experiments/data/combined')
    exp.add_fetcher('/homedtic/gfrances/projects/code/fs-sdd/experiments/data/medium-eval', filter_algorithm=algos)
    exp.add_fetcher('/homedtic/gfrances/projects/code/fs-sdd/experiments/data/medium_sok_only-eval', filter_algorithm=algos)
    exp.add_fetcher('/homedtic/gfrances/projects/code/fs-sdd/experiments/data/hard-eval', filter_algorithm=algos)

    exp.add_report(
        BaseReport(attributes=ALL_ATTRIBUTES),
        outfile='report.html')

    # Parse the commandline and run the specified steps.
    exp.run_steps()


if __name__ == '__main__':
    main()
