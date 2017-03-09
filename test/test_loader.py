
import sys
import os
import time
import importlib
import itertools
sys.path.insert(0, os.path.abspath('../preprocessor'))
import runner
from util import mkdirp

current_module = importlib.import_module(__name__)
timestamp = time.strftime("%Y%m%d_%H%M")

HOMEPATH = os.path.expanduser("~")

BENCHMARKS = {
    'fs': os.path.expanduser("~/projects/code/fs-benchmarks/benchmarks/"),
    'ipc14': os.path.expanduser("~/projects/code/ipc-2014/seq-sat/"),
    'dw': os.path.expanduser("~/projects/code/downward-benchmarks/"),
}


def create_runner(**kwargs):
    """ Creates an anonymous runner for a certain instance and planner configuration """
    tag = "test_{}".format(timestamp)
    output = os.path.abspath(os.path.join('.', 'workspace', "{}".format(tag)))
    args = ['--debug', '--run', '--tag', tag, '--output', output]

    mkdirp(output)

    mandatory = ['instance', 'driver']
    optional = ['domain', 'options']

    for argument in mandatory:
        if argument not in kwargs or kwargs[argument] is None:
            raise RuntimeError("FS requires mandatory parameter {}".format(argument))
        args += ['--{}'.format(argument), '{}'.format(kwargs[argument])]

    for argument in optional:
        if argument in kwargs and kwargs[argument] is not None:
            args += ['--{}'.format(argument), '{}'.format(kwargs[argument])]

    if 'asp' in kwargs and kwargs['asp']:
        args += ['--asp']

    def anon_runner():
        exit_code = runner.main(args)
        assert exit_code == 0

    return anon_runner


def add_run(name, run):
    setattr(current_module, "test_{}".format(name), run)


inst = os.path.join(BENCHMARKS['dw'], 'blocks', 'probBLOCKS-4-0.pddl')
drivers = ['sbfws', 'native', 'smart']

add_run('hola', create_runner(instance=inst, driver=drivers[0],
                              options="successor_generation=naive,bfws.rs=sim,evaluator_t=adaptive,goal_directed=true"))



options = []

options.append(("successor_generation", "naive,match_tree"))
options.append(("bfws.rs", "none,sim"))
options.append(("evaluator_t", "adaptive,generic"))
options.append(("goal_directed", "true,false"))





def dict_product(dicts):
    return (list(zip(dicts, x)) for x in itertools.product(*dicts.values()))


#
class ConfigManager(object):
    def __init__(self):
        self.options = dict()

    def register_option(self, name, values):
        self.options[name] = values.split(',') if isinstance(values, str) else values

    def all_configs(self):
        return list(dict_product(self.options))


def opt_str(option):
    return ','.join('{}={}'.format(name,val) for name, val in option)


mng = ConfigManager()
mng.register_option("successor_generation", "naive,match_tree")
mng.register_option("bfws.rs", "none,sim")

for i, opt in enumerate(map(opt_str, mng.all_configs()), 1):
    add_run('config_{}'.format(i), create_runner(instance=inst, driver=drivers[0], options=opt))








