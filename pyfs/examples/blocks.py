import os

from pyfs import util, runner


def main():
    instance = os.path.expanduser("~/projects/code/downward-benchmarks/blocks/probBLOCKS-4-0.pddl")
    runner.run(instance=instance)


if __name__ == "__main__":
    # Make sure that the random seed is fixed before running the script, to ensure determinism in the Python code
    if not util.fix_seed_and_possibly_rerun():
        main()
