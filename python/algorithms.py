
import itertools


def dict_product(dicts):
    return (dict(zip(dicts, x)) for x in itertools.product(*dicts.values()))
