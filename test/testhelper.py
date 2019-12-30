
import time
import os
import yaml

from python import utils, runner, algorithms, FS_WORKSPACE


import itertools


def dict_product(dicts):
    return (dict(zip(dicts, x)) for x in itertools.product(*dicts.values()))


timestamp = time.strftime("%Y%m%d_%Hh")

# Some directories where different benchmarks might reside
BENCHMARKS = {
    'fs': os.path.expanduser("~/projects/code/fs-benchmarks/benchmarks/"),
    'ipc14': os.path.expanduser("~/projects/code/ipc-2014/seq-sat/"),
    'dw': os.path.expanduser("~/projects/code/downward-benchmarks/"),
}

INSTANCES = [
    # Benchmark set, domain, instance
    ('fs', 'counters-fn', 'counters-fn/instance_5.pddl'),
    ('dw', 'blocks', 'blocks/probBLOCKS-4-0.pddl'),
    ('dw', 'gripper', 'gripper/prob01.pddl'),
    # ('dw', 'visitall-sat11-strips', 'visitall-sat11-strips/problem12.pddl'),
    # ('dw', 'sokoban-sat08-strips', 'sokoban-sat08-strips/p01.pddl'),


    # ('fs', 'counters-strips-ex', 'counters-strips-ex/instance_5.pddl'),
    # ('fs', 'simple-sokoban-fn', 'simple-sokoban-fn/instance_4_1.pddl'),


    # 'block-grouping-strips/instance_5_5_2_1.pddl',
    # 'n-puzzle-fn-v1-nested/eight01x.pddl',
    # 'blocksworld-fn-nested/instance_5_1.pddl',
    # 'graph-coloring-agent-fn/instance_5_4_1.8_1.pddl'
]


def run_planner(instance, driver, options, asp=False):
    """ Creates an anonymous runner for a certain instance and planner configuration """
    tag = "test_{}".format(timestamp)
    args = ['--debug', '--run', '--tag', tag, '--instance', instance, '--driver', driver]
    name_args = [tag, driver, os.path.basename(instance)]

    if options is not None:
        args += ['--options', options]
        name_args += [options]

    if asp:
        args += ['--asp']
        name_args += ['asp']

    exp_name = '.'.join(name_args).replace("=", '_').replace(',', '.')
    output = os.path.join(FS_WORKSPACE, exp_name)
    utils.mkdirp(output)
    args += ['--output', output]

    exit_code = runner.main(args)
    assert exit_code == 0


def opt_str(configuration):
    return ','.join('{}={}'.format(name, val) for name, val in configuration.items())


class FSTester(object):
    DRIVERS = ['sbfws', 'native', 'smart']

    def __init__(self):
        self.config = self.load_config("configs.yaml")
        self.options = self.config['options']
        self.domain_features = self.config['domain_features']

    @staticmethod
    def load_config(filename):
        with open(filename, 'r') as file:
            return yaml.load(file)

    def parametrize(self, metafunc):
        argnames = ['instance', 'options']
        argvalues = self.cross_product()
        metafunc.parametrize(argnames, argvalues)

    def list_planner_options(self):
        all_options = dict()
        for name, data in self.options.items():
            all_options[name] = list(data['values'].keys())
        return all_options

    def fits(self, domain, configuration):
        """ Return whether the given configuration first the given domain """
        domain_features = self.domain_features[domain]

        for option, value in configuration.items():
            option_properties = self.options[option]
            value_properties = option_properties['values'][value]

            # TODO - CHECK OPTION PROPERTIES ARE COMPATIBLE

            if not value_properties:
                continue

            excluded = value_properties.get('exclude_features', None)
            # included = value_properties.get('include_features', None)

            if excluded:
                for feature in excluded:
                    if domain_features.get(feature, None):
                        return False

        return True

    def cross_product(self):
        all_crossed = []
        join_options = list(dict_product(self.list_planner_options()))

        for bm_set, domain, filename in INSTANCES:
            for configuration in join_options:
                if self.fits(domain, configuration):
                    instance = os.path.join(BENCHMARKS[bm_set], filename)
                    all_crossed.append([instance, opt_str(configuration)])

        return all_crossed
