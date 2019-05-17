

from . import extension as cext


def create_model(problem, index, grounding=None, use_match_tree=False):
    model = cext.create_model(problem, use_match_tree)
    if grounding:
        compute_grounding(model, index, grounding)
    else:
        raise NotImplementedError()
    return model


def compute_grounding(model, index, grounding):
    # Declare the reachable state variables




    # Declare the reachable action groundings
    actions = grounding.ground_actions()




