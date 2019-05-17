

from . import extension as cext


def create_model(problem, index, grounding=None, use_match_tree=False):
    model = cext.create_model(problem, use_match_tree)
    if grounding:
        declare_groundings(model, index, grounding)
    else:
        raise NotImplementedError()
    return model


def declare_groundings(model, index, grounding):
    # Declare the reachable state variables
    variables = grounding.ground_state_variables()
    for i, variable in variables.enumerate():  # note that 'i' will be the "official" index of the variable
        x = 0


    # Declare the reachable action groundings
    actions = grounding.ground_actions()




