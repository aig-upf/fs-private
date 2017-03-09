
import itertools


def dict_product(dicts):
    return (list(zip(dicts, x)) for x in itertools.product(*dicts.values()))
