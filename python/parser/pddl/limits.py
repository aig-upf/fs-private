class Limits(object):
    def __init__(self, limits):
        self.limits = {}
        for limit in limits:
            domain, l = limit
            self.limits[domain] = l

    def __str__(self):
        return ", ".join(self.limits)
