"""
    A module for dealing with and storing some common string templates in a more friendly manner.
"""
import os
from string import Template

# some basic templates
_base = dict(
    function_instantiation='{"${name}", [&info](const ObjectIdxVector& parameters)'
                           '{ const External& external = static_cast<const External&>(info.get_external());  return external.${accessor}(parameters); }}',
)


class _Templates(object):
    def __init__(self, tpl_dir):
        self.tpl_dir = tpl_dir
        self.tpls = {k: Template(tpl) for k, tpl in _base.items()}

    # Returns a template object from memory or from file. If from file, caches the template for performance.
    def get(self, name):
        if name not in self.tpls:
            with open(self.tpl_dir + '/' + name + '.tpl', "r") as f:
                self.tpls[name] = Template(f.read())
        return self.tpls[name]

# We'll do away with one single instance:
currentDir = os.path.dirname(os.path.realpath(__file__))
tplDir = os.path.abspath(currentDir + '/tpls/')
tplManager = _Templates(tplDir)
