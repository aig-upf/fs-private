
class Metric : # The problem metric
    def __init__(self, optimization) :
        self.optimization = optimization
        self.expr = (None, None)

    def __str__(self) :
        return "{} {}".format(self.optimization, self.expr)

    @property
    def terminal_cost(self) :
        return self.expr[0]

    @property
    def stage_cost(self) :
        return self.expr[1]
