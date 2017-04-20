from .actions import Action

class Event(Action) :

    def __init__(self, name, parameters, num_external_parameters,
                 precondition, effects) :
        Action.__init__(self, name, parameters, num_external_parameters, precondition, effects, None)

    def __repr__(self):
        return "<Event %r at %#x>" % (self.name, id(self))

    def dump(self):
        print("%s(%s)" % (self.name, ", ".join(map(str, self.parameters))))
        print("Condition:")
        self.precondition.dump()
        print("Effects:")
        for eff in self.effects:
            eff.dump()
