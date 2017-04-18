class DomainBound(object):
    def __init__(self, typename, bound):
        self.typename = typename
        self.bound = bound
    def __str__(self):
        return "{} - {}".format(self.typename, self.bound)
