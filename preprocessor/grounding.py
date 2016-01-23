

import itertools
from extra import IndexDictionary, Atom
import base
from compilation.helper import is_external
import static


class Grounder(object):

    def __init__(self, task, compilation_index):
        self.task = task
        self.compilation_index = compilation_index

    def ground_state_variables(self, compilation_index):
        # Create an index of all possible state variables.
        compilation_index.variables = self.index_state_variables()

    def index_state_variables(self):
        task = self.task
        variables = IndexDictionary()

        for symbol in task.symbols.values():
            if not self.is_variable(symbol.name):
                continue
            instantiations = [self.task.type_map[t] for t in symbol.arguments]
            for instantiation in itertools.product(*instantiations):
                variables.index(base.Variable(symbol.name, instantiation))
        return variables

    def get_relevant_init_facts(self):
        """ Return a list of all the non-static facts in the initial state. """
        init = self.task.init
        assert isinstance(init, base.State)
        facts = []
        for name, inst in init.instantiations.items():
            # assert isinstance(inst, (base.PredicateInstantiation, base.FunctionInstantiation))
            if isinstance(inst, static.UnarySet):  # A predicate
                # symbol = self.task.domain.symbols[inst.symbol]
                # for point in self.compilation_index.objects.
                # for point in inst.set:
                #     facts.append(Fact(Variable(inst.symbol, tuple(point)), 1))
                for args in inst.elems:
                    # TODO - We should initialize the points not in the set to 0, although that'll be done
                    # TODO - by default by the compiler.
                    facts.append(Atom(base.Variable(name, args), 1))
            else:  # A function
                for args, val in inst.elems.items():
                    facts.append(Atom(base.Variable(name, args), val))
        return facts

    def is_variable(self, name):
        return not is_external(name) and name not in self.task.static_symbols

