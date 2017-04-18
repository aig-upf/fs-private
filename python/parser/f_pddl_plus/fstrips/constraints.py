class Constraint(object):
    def __init__(self, name, args, condition = None):
        self.name = name
        self.args = args
        self.condition = condition

    def __str__(self):
        return "{}({})".format(self.name, " ".join([str(arg) for arg in self.args]))
    def dump(self) :
        print("%s(%s)" % (self.name, ", ".join(map(str, self.args))))
        print('Condition:')
        self.condition.dump()
