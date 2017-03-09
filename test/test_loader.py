
import sys
import os
import time
import importlib

# Import FS python module
FS_PATH = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
sys.path.insert(0, FS_PATH)

from python import utils, runner, algorithms, FS_WORKSPACE

current_module = importlib.import_module(__name__)
timestamp = time.strftime("%Y%m%d_%H%M")

HOMEPATH = os.path.expanduser("~")

BENCHMARKS = {
    'fs': os.path.expanduser("~/projects/code/fs-benchmarks/benchmarks/"),
    'ipc14': os.path.expanduser("~/projects/code/ipc-2014/seq-sat/"),
    'dw': os.path.expanduser("~/projects/code/downward-benchmarks/"),
}


def create_runner(instance, driver, **kwargs):
    """ Creates an anonymous runner for a certain instance and planner configuration """
    tag = "test_{}".format(timestamp)
    args = ['--run', '--tag', tag, '--instance', instance, '--driver', driver]
    name_args = [tag, driver, os.path.basename(instance)]

    optional = ['domain', 'options']
    for argument in optional:
        if argument in kwargs and kwargs[argument] is not None:
            args += ['--{}'.format(argument), '{}'.format(kwargs[argument])]
            name_args += [argument, kwargs[argument]]

    if 'asp' in kwargs and kwargs['asp']:
        args += ['--asp']
        name_args += ['asp']

    exp_name = '.'.join(name_args).replace("=", '_').replace(',', '.')
    output = os.path.join(FS_WORKSPACE, exp_name)
    utils.mkdirp(output)
    args += ['--output', output]

    def _runner():
        exit_code = runner.main(args)
        assert exit_code == 0

    return _runner


def add_run(name, run):
    setattr(current_module, "test_{}".format(name), run)


inst = os.path.join(BENCHMARKS['dw'], 'blocks', 'probBLOCKS-4-0.pddl')


#
class ConfigManager(object):
    def __init__(self):
        self.options = dict()

    def register_option(self, name, values):
        self.options[name] = values.split(',') if isinstance(values, str) else values

    def all_configs(self):
        return list(algorithms.dict_product(self.options))


class FSTestManager(object):
    DRIVERS = ['sbfws', 'native', 'smart']

    def __init__(self):
        self.manager = ConfigManager()
        self.bootstrap_options()

    def bootstrap_options(self):
        self.manager.register_option("successor_generation", "naive,match_tree")
        self.manager.register_option("bfws.rs", "none,sim")
        self.manager.register_option("evaluator_t", "adaptive,generic")
        self.manager.register_option("goal_directed", "true,false")

    def add_runs(self):
        for i, opt in enumerate(map(opt_str, self.manager.all_configs()), 1):
            add_run('config_{}'.format(i), create_runner(instance=inst, driver=self.DRIVERS[0], options=opt))


def opt_str(option):
    return ','.join('{}={}'.format(name, val) for name, val in option)


mng = FSTestManager()
mng.add_runs()









