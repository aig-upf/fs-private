"""
 This file contains all the necessary entities to define state constraints
"""


class Constraint(object):
    """
    A state constraint is defined by its name, the implementation classname,
    and the state variables upon which the constraint is enforced.
    """
    def __init__(self, name, parameters, variables):
        self.name = name
        self.parameters = parameters
        self.variables = variables

    def __str__(self):
        parameters = ','.join(str(a) for a in self.parameters)
        variables = ','.join(str(a) for a in self.variables)
        return '{}[{}]({})'.format(self.name, parameters, variables)
    __repr__ = __str__


class Alldiff(Constraint):
    def __init__(self, parameters, variables):
        super().__init__("alldiff", parameters, variables)


class Sum(Constraint):
    def __init__(self, parameters, variables):
        super().__init__("sum", parameters, variables)


class External(Constraint):
    def __init__(self, name, parameters, variables):
        super().__init__(name, parameters, variables)


class GeqConstraint(Constraint):
    def __init__(self, parameters, variables):
        super().__init__("geq", parameters, variables)


class ConstraintCatalog(object):
    """ A catalog of custom constraints """
    supported = {'sum': Sum, 'alldiff': Alldiff, ">=": GeqConstraint}

    @classmethod
    def is_supported(cls, name):
        return name in cls.supported

    @classmethod
    def instantiate(cls, name, parameters, variables):
        return cls.supported[name](parameters, variables)