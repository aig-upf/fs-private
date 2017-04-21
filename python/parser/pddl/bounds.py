class DomainBound(object):
    def __init__(self, typename, bound):
        self.typename = typename
        self.bound = bound
    def __str__(self):
        return "{} - {}".format(self.typename, self.bound)

def parse_bounds(bounds):
    parsed = []
    for bound in bounds:
        assert len(bound) == 3 and bound[1] == '-'  # bound is of the form ["val", "-", "int[0..10]"]
        parsed.append(DomainBound(bound[0], bound[2]))
    return parsed
