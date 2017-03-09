from __future__ import print_function

def parse_expression(exp):
    if isinstance(exp, list):
        functionsymbol = exp[0]
        return PrimitiveNumericExpression(functionsymbol, exp[1:])
    elif exp.replace(".", "").isdigit():
        return NumericConstant(float(exp))
    elif exp[0] == "-":
        return NumericConstant(float(exp))
        #raise ValueError("Negative numbers are not supported")
    else:
        return PrimitiveNumericExpression(exp, [])

def parse_assignment(alist):
    assert len(alist) == 3
    op = alist[0]
    head = parse_expression(alist[1])
    exp = parse_expression(alist[2])
    if op == "=":
        return Assign(head, exp)
    elif op == "increase":
        return Increase(head, exp)
    elif op == "decrease":
    	return Decrease(head, exp)
    else:
        assert False, "Assignment operator not supported."

def parse_functional_assignment(alist):
    """ Parses a functional assignment, e.g. (assign (f ?a ?b) (g ?c)), i.e. f(a,b) := g(c) """
    assert len(alist) == 3
    assert alist[0] == "assign" # So far we only support assignations
    head = parse_term(alist[1])
    exp = parse_term(alist[2])
    return head, exp

def parse_term(exp):
    if isinstance(exp, list):  # We have a functional term
        function_symbol = exp[0]
        return FunctionalTerm(function_symbol, parse_term_list(exp[1:]))

    else:  # exp is a string and we have a non-functional term
        if exp[0] == '?':  # We have a variable
            return exp
        else:  # we must have a constant
            return exp  # ATM No differentiation

def parse_term_list(term_list):
    assert isinstance(term_list, list)
    return [parse_term(t) for t in term_list]

class FunctionalTerm(object):
    def __init__(self, symbol, args):
        self.symbol = symbol
        self.args = tuple(args)
        self.hash = hash((self.__class__, self.symbol,self.args))

    def __hash__(self):
        return self.hash

    def __str__(self):
        return "{self.symbol}({args})".format(self=self, args=', '.join(map(str, self.args)))

    __repr__ = __str__



class FunctionalExpression(object):
    def __init__(self, parts):
        self.parts = tuple(parts)
    def dump(self, indent="  "):
        print("%s%s" % (indent, self._dump()))
        for part in self.parts:
            part.dump(indent + "  ")
    def _dump(self):
        return self.__class__.__name__
    def instantiate(self, var_mapping, init_facts):
        raise ValueError("Cannot instantiate condition: not normalized")

class NumericConstant(FunctionalExpression):
    parts = ()
    def __init__(self, value):
        if value != int(value):
            raise ValueError("Fractional numbers are not supported")
        self.value = int(value)
    def __eq__(self, other):
        return (self.__class__ == other.__class__ and self.value == other.value)
    def __str__(self):
        return "%s %s" % (self.__class__.__name__, self.value)
    def _dump(self):
        return str(self)
    def instantiate(self, var_mapping, init_facts):
        return self

class PrimitiveNumericExpression(FunctionalExpression):
    parts = ()
    def __init__(self, symbol, args):
        self.symbol = symbol
        self.args = tuple([ parse_term(arg) for arg in args])
        self.hash = hash((self.__class__, self.symbol, self.args))
    def __hash__(self):
        return self.hash
    def __eq__(self, other):
        return (self.__class__ == other.__class__ and self.symbol == other.symbol
                and self.args == other.args)
    def __str__(self):
        return "%s %s(%s)" % ("PNE", self.symbol, ", ".join(map(str, self.args)))
    def dump(self, indent="  "):
        print("%s%s" % (indent, self._dump()))
        for arg in self.args:
            arg.dump(indent + "  ")
    def _dump(self):
        return str(self)
    def instantiate(self, var_mapping, init_facts):
        args = [var_mapping.get(arg, arg) for arg in self.args]
        pne = PrimitiveNumericExpression(self.symbol, args)
        assert self.symbol != "total-cost"
        # We know this expression is constant. Substitute it by corresponding
        # initialization from task.
        for fact in init_facts:
            if isinstance(fact, FunctionAssignment):
                if fact.fluent == pne:
                    return fact.expression
        assert False, "Could not find instantiation for PNE!"

#class PrimitiveObjectualExpression(FunctionalExpression):
#    parts = ()
#    def __init__(self, symbol, args):
#        self.symbol = symbol
#        self.args = tuple(args)
#        self.hash = hash((self.__class__, self.symbol, self.args))
#    def __hash__(self):
#        return self.hash
#    def __eq__(self, other):
#        return (self.__class__ == other.__class__ and self.symbol == other.symbol
#                and self.args == other.args)
#    def __str__(self):
#        return "%s %s(%s)" % ("POE", self.symbol, ", ".join(map(str, self.args)))
#    def dump(self, indent="  "):
#        print("%s%s" % (indent, self._dump()))
#        for arg in self.args:
#            arg.dump(indent + "  ")
#    def _dump(self):
#        return str(self)
#    def instantiate(self, var_mapping, init_facts):
#        raise RuntimeError("Not yet implemented") # @see PrimitiveNumericExpression::instantiate


class FunctionAssignment(object):
    def __init__(self, fluent, expression):
        self.fluent = fluent
        self.expression = expression
    def __str__(self):
        return "%s %s %s" % (self.__class__.__name__, self.fluent, self.expression)
    def dump(self, indent="  "):
        print("%s%s" % (indent, self._dump()))
        self.fluent.dump(indent + "  ")
        self.expression.dump(indent + "  ")
    def _dump(self):
        return self.__class__.__name__
    def instantiate(self, var_mapping, init_facts):
        if not (isinstance(self.expression, PrimitiveNumericExpression) or
                isinstance(self.expression, NumericConstant)):
            raise ValueError("Cannot instantiate assignment: not normalized")
        # We know that this assignment is a cost effect of an action (for initial state
        # assignments, "instantiate" is not called). Hence, we know that the fluent is
        # the 0-ary "total-cost" which does not need to be instantiated
        assert self.fluent.symbol == "total-cost"
        fluent = self.fluent
        expression = self.expression.instantiate(var_mapping, init_facts)
        return self.__class__(fluent, expression)

class Assign(FunctionAssignment):
    def __str__(self):
        return "%s := %s" % (self.fluent, self.expression)

class Increase(FunctionAssignment):
    pass

class Decrease(FunctionAssignment):
    pass
