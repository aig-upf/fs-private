
# Import all symbols from the C++ extension
from ._pyfs import *
from . import engines, heuristics

import os
from driver import util, run_components as fddriver
from .core import Options

MODULE_PATH = os.path.dirname(os.path.abspath(__file__))
BUILD_PATH = os.path.abspath(os.path.join(MODULE_PATH, '..',))

# print("pyfs module successfully imported")


def load(instance, domain=None):
    domain = domain or util.find_domain_filename(instance)

    # Generate the output.sas file
    call_translator(domain, instance)

    # TODO This should be temporary until a better way of specifying the output.sas file makes it into the default branch
    OUTPUT_SAS = os.path.abspath(os.path.join(BUILD_PATH, '..', '..', 'output.sas'))
    assert os.path.exists(OUTPUT_SAS)
    initialize(OUTPUT_SAS)


def call_translator(domain, instance):
    translator = fddriver.get_executable(os.path.join(BUILD_PATH, 'bin'),
                                         fddriver.REL_TRANSLATE_PATH)
    fddriver.call_component(translator, [domain, instance])


def create_engine(name, **kwargs):
    """ Create an engine with given name and options """
    return dispatch_to_module(name, engines, "engine", **kwargs)


def create_heuristic(name, **kwargs):
    """ Create an heuristic with given name and options """
    return dispatch_to_module(name, heuristics, "heuristic", **kwargs)


def dispatch_to_module(name, module_, object_name, **kwargs):
    """
        Helper to dispatch the requests to to the appropriate method in the given module
    """
    creator = "create_{}".format(name)
    if not hasattr(module_, creator):
        raise RuntimeError("Unknown {} type '{}'".format(object_name, name))

    return getattr(module_, creator)(**kwargs)
