"""
    Grounding of Tarski problems
"""
import logging
import shutil

from tarski.grounding import LPGroundingStrategy

from . import extension as cext


def lpgrounding(problem, language_info):
    """ Return a Tarski LP-based grounding strategy """
    if shutil.which("gringo") is None:
        raise RuntimeError('Install the Clingo ASP solver and put the "gringo" binary on your PATH in order to run '
                           'the requested ASP-based reachability analysis')
    cgrounding = cext.Grounding(language_info.linfo)

    grounding = LPGroundingStrategy(problem)
    variables = grounding.ground_state_variables()

    for i, variable in variables.enumerate():  # note that 'i' will be the "official" index of the variable
        cext.add_state_variable(cgrounding,
                                language_info.symbol_idxs[variable.symbol],
                                [language_info.obj_idxs[o] for o in variable.binding]
                                )
    return cgrounding
