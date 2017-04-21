
class Metric : # The problem metric
    def __init__(self, optimization) :
        self.optimization = optimization
        self.expr = None

    def __str__(self) :
        return "{} {}".format(self.optimization, self.expr)
