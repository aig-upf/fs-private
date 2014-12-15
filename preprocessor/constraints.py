"""
 This file contains all the necessary entities to define state constraints
"""


class Constraint(object):
    """
    A state constraint is defined by its name, the implementation classname,
    and the state variables upon which the constraint is enforced.
    """
    def __init__(self, name, args):
        self.name = name
        self.args = args

    def __str__(self):
        return '{}({})'.format(self.name, ','.join(str(a) for a in self.args))
    __repr__ = __str__


class Alldiff(Constraint):
    def __init__(self, args):
        super().__init__("alldiff", args)

    def getArgumentTypes(self, arguments):
        return


class Sum(Constraint):
    def __init__(self, args):
        super().__init__("sum", args)