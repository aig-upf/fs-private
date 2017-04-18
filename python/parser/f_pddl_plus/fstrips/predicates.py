from . import fs_types

built_in_predicative_symbols = ['=', '>', '<', '>', '<=', '>=']

class Predicate(object):

    def __init__(self, name, arguments):
        self.name = name
        self.arguments = arguments

    def __str__(self):
        return "%s(%s)" % (self.name, ", ".join(map(str, self.arguments)))
