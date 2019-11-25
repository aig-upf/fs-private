"""
    Grounding of Tarski problems
"""
import logging
import shutil

from tarski.grounding import LPGroundingStrategy
from tarski.syntax import symref

from . import extension as cext


def lpgrounding(problem, cproblem, language_info):
    """ Return a Tarski LP-based grounding strategy """
    if shutil.which("gringo") is None:
        raise RuntimeError('Install the Clingo ASP solver and put the "gringo" binary on your PATH in order to run '
                           'the requested ASP-based reachability analysis')
    cgrounding = cext.Grounding(language_info.linfo, cproblem)

    grounding = LPGroundingStrategy(problem)
    variables = grounding.ground_state_variables()

    for i, variable in variables.enumerate():  # note that 'i' will be the "official" index of the variable
        cgrounding.add_state_variable(
            language_info.symbol_idxs[variable.symbol],
            [language_info.obj_idxs[symref(o)] for o in variable.binding]
        )

    # TODO: Create the ground actions
    actions = grounding.ground_actions()
    # print(actions)
    # for schema_name, schema_groundings in actions.items():

    return cgrounding
