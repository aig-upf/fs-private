
class TransitionEdge(object):
    def __init__(self, variable, v1, v2):
        self.variable = variable
        self.v1 = v1
        self.v2 = v2

    def __repr__(self):
        return "{}: {} -> {}".format(self.variable, self.v1, self.v2)


def parse_transitions(transitions):
    parsed = []
    for transition in transitions:
        assert len(transition) == 3  # transition is of the form [["f", "c"], "v1", "v2"]
        parsed.append(TransitionEdge(*transition))
    return parsed
