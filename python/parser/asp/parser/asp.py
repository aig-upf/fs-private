"""
    Some routines to write out PDDL-based logic programs
"""

import os


def _ensure_list(elem):
    return elem if isinstance(elem, list) else [elem]


class Rule(object):
    def __init__(self, head, body=None):
        self.head = head
        self.body = body

    def is_fact(self):
        return self.body is None

    def __str__(self):
        if self.is_fact():
            return "{}.".format(self.head)
        else:
            return "{} :- {}.".format(self.head, self.body)


class Translator(object):
    def __init__(self):
        pass

    def atom(self, symbol, arguments):
        arglist = ", ".join(str(arg) for arg in _ensure_list(arguments))
        return "{}({})".format(symbol, arglist)

    def type(self, t, var):
        if t.parent is None:
            return None
        return Rule(self.atom(t.parent.asp_name, var), self.atom(t.asp_name, var))

    def object(self, o):
        return Rule(self.atom(o.otype.asp_name, o.asp_name))

    def action_head(self, action):
        # TODO
        pass


class FileWriter(object):
    def __init__(self, filename):
        self.filename = filename
        if not os.path.exists(os.path.dirname(filename)):
            os.makedirs(os.path.dirname(filename))
        print('Writing LP into file "{}"'.format(filename))
        self.fd = open(filename, 'w')

    def desc(self):
        return self.fd

    def close(self):
        self.fd.close()

    def writeln(self, line):
        self.fd.write(str(line) + "\n")

    def write_str(self, line):
        self.fd.write(str(line))

    def write(self, elements):
        for elem in elements:
            self.writeln(elem)

