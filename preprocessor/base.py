"""
 This file contains all the necessary entities to define P3R domains and problems.
"""
from collections import OrderedDict
import operator
from compilation.exceptions import ParseException
from util import is_int, is_action_parameter


class Variable(object):
    """
    A state variable, made up of a symbol and a number of arguments.
    """

    def __init__(self, symbol, args):
        self.symbol = symbol
        self.args = tuple(int(a) if is_int(a) else a for a in args)

    # def is_lifted(self):
    # return any(is_action_parameter(arg) for arg in self.args)

    def __hash__(self):
        return hash((self.symbol, self.args))

    def __eq__(self, other):
        return (self.symbol, self.args) == (other.symbol, other.args)

    def __str__(self):
        return '{}{}'.format(self.symbol, "(" + ', '.join(map(str, self.args)) + ")")

    __repr__ = __str__

    def ground(self, binding):
        args = []
        for a in self.args:
            if is_action_parameter(a):
                args.append(binding[a])
            else:  # Assume it is a constant
                args.append(a)
        return Variable(self.symbol, args)


class UngroundedVariable(Variable):
    pass


class ProblemObject(object):
    def __init__(self, name, typename):
        self.name = name
        self.typename = typename


class ObjectType(object):
    def __init__(self, name, parent):
        self.name = name
        self.parent = parent


class Symbol(object):
    def __init__(self, name, arguments):
        self.name = name
        self.arguments = arguments


class Predicate(Symbol):
    def __init__(self, name, domain):
        super().__init__(name, domain)


class Function(Symbol):
    def __init__(self, name, domain, codomain):
        super().__init__(name, domain)
        self.codomain = codomain


class FunctionInstantiation(object):
    def __init__(self, symbol, mapping=None):
        self.symbol = symbol
        self.mapping = mapping if mapping else {}

    def add(self, point, value):
        assert not point in self.mapping
        self.mapping[point] = value


class PredicateInstantiation(object):
    def __init__(self, symbol, mapping=None):
        self.symbol = symbol
        self.set = mapping if mapping else set()

    def add(self, point):
        assert not point in self.set
        self.set.add(point)


class Parameter(object):
    def __init__(self, name, typename):
        self.name = name
        self.typename = typename

    def __str__(self):
        return '{}: {}'.format(self.name, self.typename)


class AppProcedure(object):
    TYPE = 'CONSTRAINT'

    def __init__(self, name, variables, code, comment="", builtin=None):
        self.name = name
        self.comment = comment
        self.variables = variables
        self.code = code
        self.builtin = builtin

    def process_component(self, symbol_map):
        code = []
        if not self.builtin:
            code = self.get_comments() + [self.templatize_code(symbol_map)]

        return ProcessedComponent(self.name, code, self.TYPE, self.builtin, len(self.variables))

    def get_comments(self):
        return ['// ' + self.name] + (['// ' + self.comment] if self.comment else [])

    def templatize_code(self, symbol_map):
        return self.code.tpl(symbol_map) if isinstance(self.code, Code) else self.code


class EffProcedure(AppProcedure):
    TYPE = 'EFFECT'

    def __init__(self, name, relevant_variables, affected_variables, code, comment="", builtin=None):
        super().__init__(name, relevant_variables, code, comment, builtin)
        if len(affected_variables) != 1:
            raise RuntimeError("Currently only effects that affect one single variable are accepted")
        self.affected_variables = affected_variables


class StaticAppProcedure(object):
    def __init__(self, condition, comment=""):
        assert isinstance(condition, StaticRoutine)
        self.condition = condition
        self.comment = comment


class StaticRoutine(object):
    """ A Static Routine is some kind of routine that can be statically computed during grounding time """

    def __init__(self, routine, parameters):
        self.routine = routine
        self.parameters = parameters

    def execute(self, static, binding):
        """ Executes the static routine extracting the relevant parameters from a parameter binding. """
        # params = [binding[param] for param in self.parameters]
        # return self.routine(*params)
        # assert all(isinstance(a, ParameterExpression) for a in subexp.arguments)
        args = tuple(binding[a.symbol] for a in self.parameters)
        data = static[self.routine].elems
        return args in data  # TODO - GENERALIZE FOR FUNCTIONS AS WELL ?

    def compute_parameters(self, parameters, binding):
        computed = []
        for p in parameters:
            if isinstance(p, ParameterExpression):
                computed.append(binding[p.symbol])
            elif isinstance(p, StaticFunctionalExpression):
                routine = StaticRoutine(p.symbol, p.args)
                assert False

        return tuple(computed)


class StaticFunctionalRoutine(StaticRoutine):
    BUILTIN = {"+": operator.add, "-": operator.sub}

    def execute(self, static, binding):
        """ Executes the static routine extracting the relevant parameters from a parameter binding. """
        # params = [binding[param] for param in self.parameters]
        # return self.routine(*params)
        assert all(isinstance(a, ParameterExpression) for a in self.parameters)
        args = tuple(binding[a.symbol] for a in self.parameters)
        if self.routine in self.BUILTIN:
            return self.BUILTIN[self.routine](*args)
        else:
            data = static[self.routine].elems
            return data[args] if args in data else None


class StaticPredicativeRoutine(StaticRoutine):
    def __init__(self, routine, negated, parameters):
        super().__init__(routine, parameters)
        self.negated = negated


class StaticRelationalRoutine(StaticPredicativeRoutine):
    BUILTIN = {"=": operator.eq, "<=": operator.le, "<": operator.lt, ">=": operator.ge, ">": operator.gt}

    def execute(self, static, binding):
        """ Executes the static routine extracting the relevant parameters from a parameter binding. """
        # params = [binding[param] for param in self.parameters]
        # return self.routine(*params)
        # assert all(isinstance(a, ParameterExpression) for a in subexp.arguments)
        args = self.compute_parameters(self.parameters, static, binding)
        assert self.routine in self.BUILTIN and len(args) == 2
        res = self.BUILTIN[self.routine](*args)
        return not res if self.negated else res

    def compute_parameters(self, parameters, static, binding):
        computed = []
        for p in parameters:
            if isinstance(p, ParameterExpression):
                computed.append(binding[p.symbol])
            elif isinstance(p, StaticFunctionalExpression):
                routine = StaticFunctionalRoutine(p.symbol, p.arguments)
                computed.append(routine.execute(static, binding))
            elif isinstance(p, NumericExpression):
                computed.append(int(p.symbol))
            else:
                raise RuntimeError("Unimplemented")

        return tuple(computed)


class DefinedRoutine(StaticPredicativeRoutine):
    def __init__(self, expression):
        self.expression = expression
        super().__init__(None, expression.negated, None)

    def execute(self, static, binding):
        """ Executes the static routine extracting the relevant parameters from a parameter binding. """
        # TODO - GENERALIZE
        subexp = self.expression.arguments[0]
        # assert all(isinstance(a, ParameterExpression) for a in subexp.arguments)
        args = tuple(binding[a.symbol] for a in subexp.arguments)
        data = static[subexp.symbol].elems
        defined = args in data
        return not defined if self.expression.negated else defined


class Code(object):
    """
        A Code object contains a with C++ code that possibly needs to be instantiated with the actual variable
        indexes that correspond to parameters, etc. of the action where the code is to be used.
    """

    def __init__(self, code):
        assert isinstance(code, str)
        self.code = code

    def tpl(self, symbol_map):
        return self.code.format(**symbol_map)


class ApplicableEntity(object):
    def __init__(self, name, parameters):
        self.name = name
        self.parameters = parameters
        self.applicability_procedures = []
        self.static_applicability_procedures = []
        self.param_index = {}
        self.defines = []
        self.define_index = {}
        self.parameter_map = {}
        self.param_names = []
        self.index_parameters()

    def add_applicability_procedure(self, procedure):
        if isinstance(procedure, AppProcedure):
            self.check_procedure_variables(procedure)
            self.applicability_procedures.append(procedure)
        elif isinstance(procedure, StaticAppProcedure):
            self.static_applicability_procedures.append(procedure)
        else:
            raise RuntimeError("Unknown procedure type for '{}'".format(procedure))

    def add_static_applicability_procedure(self, procedure):
        return self.add_applicability_procedure(procedure)

    def index_parameters(self):
        for i, param in enumerate(self.parameters):
            if param.name in self.param_index:
                raise Exception("Action '{}' contains two parameters with name '{}'".format(self.name, param.name))
            self.parameter_map[param.name] = "_binding[{}]".format(i)
            self.param_index[param.name] = param.typename

    def define(self, name, routine):
        """ Defines a derived value to be used during the definition of the action """
        assert isinstance(routine, StaticRoutine)
        assert name not in self.define_index and name not in self.param_index, \
            "A defined symbol with name '{}' has already been defined".format(name)
        self.define_index[name] = routine
        self.defines.append(name)
        self.parameter_map[name] = "_derived[{}]".format(len(self.defines) - 1)

    def has_defined_symbols(self):
        return bool(self.defines)

    def is_valid_symbol(self, name):
        """ Returns true iff the symbol is a valid reference to a parameter or defined value of the entity. """
        return name in self.param_index or name in self.define_index

    def check_procedure_variables(self, procedure):
        """
        Checks that all the (ungrounded) variables of the procedure refer to either a parameter of the entity
        or to some defined derived value.
        """
        for var in procedure.variables:
            if isinstance(var, UngroundedVariable):
                for arg in var.args:
                    if not self.is_valid_symbol(arg):
                        raise Exception("Procedure '{}' from action '{}' contains an unresolved reference"
                                        " to parameter '{}'".format(procedure.name, self.name, arg))


class Action(ApplicableEntity):
    def __init__(self, name, parameters):
        super().__init__(name, parameters)
        self.effect_procedures = []

    def add_effect_procedure(self, procedure):
        assert (isinstance(procedure, EffProcedure))
        self.effect_procedures.append(procedure)

    def __str__(self):
        parameters = ', '.join(str(p) for p in self.parameters)
        return 'Action {}({})'.format(self.name, parameters)


class GroundedApplicableEntity(object):
    def __init__(self, name, binding=None, derived=None, applicability_procedures=None):
        self.name = name
        self.binding = binding if binding else []
        self.derived = derived if derived else []
        self.applicability_procedures = applicability_procedures if applicability_procedures else []

    def add_applicability_procedure(self, procedure):
        assert (isinstance(procedure, AppProcedure))
        self.applicability_procedures.append(procedure)


class GroundedAction(GroundedApplicableEntity):
    def __init__(self, name, applicability_procedures=None, effect_procedures=None, binding=None, derived=None):
        super().__init__(name, binding, derived, applicability_procedures)
        self.effect_procedures = effect_procedures if effect_procedures else []

    def add_effect_procedure(self, procedure):
        assert (isinstance(procedure, EffProcedure))
        self.effect_procedures.append(procedure)

    def __str__(self):
        parameters = ', '.join(str(p) for p in self.binding)
        return '{}({})'.format(self.name, parameters)

    @staticmethod
    def create_from_lifted_action(task, action, binding):
        """
            Factory method to create a grounded action from a lifted action and a binding scheme.
            Returns None if the grounded action contains some static procedure that evaluates to
            false for the given binding.
        """
        assert isinstance(action, Action)

        # Check the static applicability procedures to discard unapplicable groundings
        for proc in action.static_applicability_procedures:
            if not proc.condition.execute(task.static_data, binding):
                return None

        # Ground the applicability procedures
        app_procedures = []
        for proc in action.applicability_procedures:
            variables = GroundedAction.ground_variables(proc.variables, binding)
            app_procedures.append(AppProcedure(proc.name, variables, proc.code))

        # Ground the effect procedures
        eff_procedures = []
        for proc in action.effect_procedures:
            rel_vars = GroundedAction.ground_variables(proc.variables, binding)
            aff_vars = GroundedAction.ground_variables(proc.affected_variables, binding)
            eff_procedures.append(EffProcedure(proc.name, rel_vars, aff_vars, proc.code))

        ordered_binding = [binding[p.name] for p in action.parameters]
        ordered_derived = [binding[s] for s in action.defines]
        return GroundedAction(action.name, app_procedures, eff_procedures, ordered_binding, ordered_derived)

    @staticmethod
    def ground_variables(variables, binding):
        """ Ground the given list of variables with respect to the given binding. """
        grounded = []
        for var in variables:
            grounded.append(var.ground(binding))
            if not isinstance(var, (UngroundedVariable, Variable)):
                raise RuntimeError("Unkown variable type; '{}'".format(var))
        return grounded


class Goal(GroundedApplicableEntity):
    def __init__(self):
        super().__init__("_goal_")


class State(object):
    def __init__(self, instantiations):
        self.instantiations = instantiations


class ProblemDomain(object):
    def __init__(self, name, types, symbols, actions):
        self.name = name
        self.types = types
        self.type_info = self.index_by_name(types)
        self.symbols = self.index_by_name(symbols)
        self.actions = actions

    def index_by_name(self, objects):
        """ Index the given objects by their name """
        ordered = OrderedDict()
        for obj in objects:
            ordered[obj.name] = obj
        return ordered

    def get_predicates(self):
        """ Small helper to iterate through the predicates """
        return (s for s in self.symbols.values() if isinstance(s, Predicate))

    def is_predicative(self):
        """ A domain is predicative if it has no functions, only predicates"""
        return all(isinstance(s, Predicate) for s in self.symbols.values())


class ProblemInstance(object):
    def __init__(self, name, domain, objects, init, goal, static_data, constraints, gconstraints):
        self.name = name
        self.domain = domain
        self.objects = objects
        self.init = init
        self.goal = goal
        self.static_data = static_data
        self.constraints = constraints
        self.gconstraints = gconstraints

    def get_complete_name(self):
        return self.domain.name + '/' + self.name


def bool_string(value):
    return '_true_' if value else '_false_'


class Expression(object):
    def __init__(self, symbol, arguments=None):
        self.symbol = symbol
        self.arguments = arguments if arguments else []

        if self.is_fluent() and not self.is_subtree_static():
            raise ParseException("Unsupported nested expression '{}'".format(self))

    def __str__(self):
        args = "" if self.arguments is None else "({})".format(', '.join(map(str, self.arguments)))
        return "{}{}".format(self.symbol, args)

    def is_fluent(self):
        return not self.is_static()

    def is_static(self):
        return False  # By default, expressions are not static

    def is_subtree_static(self, arguments=None):
        arguments = (self.arguments if self.arguments is not None else []) if arguments is None else arguments
        for arg in arguments:
            if isinstance(arg, (FunctionalExpression, VariableExpression)):
                if not arg.is_static() or not self.is_subtree_static(arg.arguments):
                    return False
        return True

    def is_tree_static(self):
        return self.is_static() and self.is_subtree_static()

    def consolidate_variables(self):
        """
        Traverses the parse tree and substitutes all those fluent functional or predicative expressions by their
        corresponding State Variables.
        Returns a tuple with all the variables found and the new tree.
        """
        relevant = []
        var = self.compute_relevant_variables(relevant)
        return (var if var is not None else self), relevant

    def compute_relevant_variables(self, relevant):
        return None

    def _compute_static_relevant_variables(self, relevant):
        for i, arg in enumerate(self.arguments):
            var = arg.compute_relevant_variables(relevant)
            if var is not None:
                self.arguments[i] = var  # Replace the previous node with the new Variable node
        return None

    def _compute_fluent_relevant_variables(self, relevant):
        var = Variable(self.symbol, [a.symbol for a in self.arguments])
        relevant.append(var)
        return VariableExpression(var)


class FunctionalExpression(Expression):
    def compute_relevant_variables(self, relevant):
        return self._compute_fluent_relevant_variables(relevant)


class StaticFunctionalExpression(FunctionalExpression):
    def is_static(self):
        return True

    def compute_relevant_variables(self, relevant):
        return self._compute_static_relevant_variables(relevant)


class PredicativeExpression(Expression):
    def __init__(self, symbol, negated, arguments=None):
        super().__init__(symbol, arguments)
        self.negated = negated

    def compute_relevant_variables(self, relevant):
        return self._compute_fluent_relevant_variables(relevant)

    def __str__(self):
        p = Expression.__str__(self)
        return '{}{}'.format("not " if self.negated else "", p)


class StaticPredicativeExpression(PredicativeExpression):
    def is_static(self):
        return True

    def compute_relevant_variables(self, relevant):
        return self._compute_static_relevant_variables(relevant)


class ConstraintExpression(StaticPredicativeExpression):
    def __init__(self, symbol, arguments):
        super().__init__(symbol, False, arguments)


class RelationalExpression(StaticPredicativeExpression):
    def __init__(self, symbol, negated, arguments):
        assert len(arguments) == 2
        super().__init__(symbol, negated, arguments)


class ArithmeticExpression(StaticFunctionalExpression):
    def __init__(self, symbol, arguments):
        assert len(arguments) == 2
        super().__init__(symbol, arguments)


class DefinedExpression(StaticPredicativeExpression):
    def __init__(self, negated, expression):
        super().__init__("defined", negated, [expression])


class VariableExpression(Expression):
    def __init__(self, variable):
        assert isinstance(variable, Variable)
        super().__init__(variable.symbol, None)
        self.variable = variable

    def __str__(self):
        return str(self.variable)


class ParameterExpression(Expression):
    def __init__(self, name):
        super().__init__(name)


class ObjectExpression(Expression):
    pass


class NumericExpression(Expression):
    pass


class EQConstraintExpression(ConstraintExpression):
    codename = "EQConstraint"


class NEQConstraintExpression(ConstraintExpression):
    codename = "NEQConstraint"

    def __init__(self, arguments, parameters):
        super().__init__('!=', arguments)
        self.parameters = parameters


class EQXConstraintExpression(ConstraintExpression):
    codename = "EQXConstraint"

    def __init__(self, arguments, parameters):
        super().__init__('=', arguments)
        self.parameters = parameters


class NEQXConstraintExpression(ConstraintExpression):
    codename = "NEQXConstraint"


class LTConstraintExpression(ConstraintExpression):
    codename = "LTConstraint"


class LEQConstraintExpression(ConstraintExpression):
    codename = "LEQConstraint"


class SumConstraintExpression(ConstraintExpression):
    codename = "ScopedSumConstraint"


class AlldiffConstraintExpression(ConstraintExpression):
    codename = "ScopedAlldiffConstraint"


class EffectExpression(object):
    def __init__(self, scope, affected, parameters=None):
        parameters = parameters if parameters is not None else []
        self.scope = scope
        self.affected = affected
        self.parameters = parameters


class AdditiveUnaryEffectExpression(EffectExpression):
    codename = "AdditiveUnaryEffect"


class ValueAssignmentEffectExpression(EffectExpression):
    codename = "ValueAssignmentEffect"


class VariableAssignmentEffectExpression(EffectExpression):
    codename = "VariableAssignmentEffect"


class ConstraintExpressionCatalog(object):
    """ A catalog of possible builtin precondition / goal constraints """
    supported = {'<': LTConstraintExpression, '<=': LEQConstraintExpression}
    supported_inv = {">=": LEQConstraintExpression, ">": LTConstraintExpression}
    supported_neg = {'>=': LTConstraintExpression, ">": LEQConstraintExpression}
    supported_neg_inv = {'<': LEQConstraintExpression, '<=': LTConstraintExpression}

    @classmethod
    def instantiate(cls, expression, index, printer):
        s, args = expression.symbol, expression.arguments
        scope, binding, parameters = classify_arguments(args, index, printer)

        if len(args) != 2:
            return None

        # Handle first equality constraints
        if s == '=':  # (in) equalities are symmetric
            assert len(scope) in (1, 2)
            if len(scope) == 2:
                if not expression.negated:
                    return EQConstraintExpression('=', args)
                else:
                    return NEQConstraintExpression('=', args)
            elif len(scope) == 1:
                if not expression.negated:
                    return EQXConstraintExpression(args, parameters)
                else:
                    return NEQXConstraintExpression(args, parameters)

        # Now for inequality constraints
        if not expression.negated:
            if s in cls.supported:
                return cls.supported[s](s, args)
            elif s in cls.supported_inv:  # We need to invert the argument list
                return cls.supported_inv[s](s, [args[1], args[0]])

        else:
            if s in cls.supported_neg:
                return cls.supported_neg[s](s, args)
            elif s in cls.supported_neg_inv:  # We need to invert the argument list
                return cls.supported_neg_inv[s](s, [args[1], args[0]])

        return None

    @classmethod
    def instantiate_custom_constraint(cls, name, args):
        custom = {"sum_constraint": SumConstraintExpression, "alldiff_constraint": AlldiffConstraintExpression}
        assert name in custom
        return custom[name](name, args)


class EffectExpressionCatalog(object):
    """ A catalog of possible builtin effect constraints """

    @classmethod
    def instantiate(cls, procedure, index, printer, expression):
        affected = procedure.affected_variables
        s, args = expression.symbol, expression.arguments
        scope, binding, parameters = classify_arguments(args, index, printer)
        if isinstance(expression, ArithmeticExpression):
            if s == '+':
                return AdditiveUnaryEffectExpression(scope, affected, parameters)
            elif s == '-':
                parameters = [-1 * p for p in parameters]
                return AdditiveUnaryEffectExpression(scope, affected, parameters)
            else:
                raise RuntimeError("To implement")

        elif isinstance(expression, ParameterExpression):
            assert len(parameters) == 0 and len(scope) == 0
            parameters = [index.parameters[expression.symbol]]
            return ValueAssignmentEffectExpression(scope, affected, parameters)

        elif isinstance(expression, ObjectExpression):
            parameters = [index.object_idx[expression.symbol]]
            return ValueAssignmentEffectExpression(scope, affected, parameters)

        elif isinstance(expression, VariableExpression):
            return VariableAssignmentEffectExpression(scope, affected, parameters)

        return None


def classify_arguments(arguments, index, printer):
    scope = []
    binding = []
    parameters = []
    for arg in arguments:
        if isinstance(arg, VariableExpression):
            scope.append(arg)

        elif isinstance(arg, NumericExpression):
            parameters.append(int(arg.symbol))

        elif isinstance(arg, ParameterExpression):
            parameters.append(index.parameters[arg.symbol])

        elif isinstance(arg, ObjectExpression):
            parameters.append(index.object_idx[arg.symbol])

        elif isinstance(arg, StaticFunctionalExpression):
            if arg.is_tree_static():
                parameters.append(printer.print(arg))
            else:
                scope.append(printer.print(arg))

        else:
            raise RuntimeError("Unknown argument")
    return scope, binding, parameters


class ProcessedComponent(object):
    """ A processed component contains the relevant code and information for a processed constraint / effect """

    def __init__(self, name, code, _type, builtin, arity):
        assert isinstance(code, list)
        self.name = name
        self.arity = arity
        self._type = _type
        self.builtin = builtin
        self.code = self.optimize_code(code, arity)

    def get_baseclass(self):
        if self._type == 'EFFECT':
            classnames = {0: 'ZeroaryScopedEffect', 1: 'UnaryScopedEffect', 2: 'BinaryScopedEffect'}
            return classnames[self.arity] if self.arity in classnames else 'ScopedEffect'
        else:
            classnames = {1: 'UnaryParametrizedScopedConstraint', 2: 'BinaryParametrizedScopedConstraint'}
            return classnames[self.arity]

    def get_satisfied_header(self):
        if self._type == 'EFFECT':
            return ''
        else:
            classnames = {1: 'satisfied_unary_header', 2: 'satisfied_binary_header'}
            return classnames[self.arity] if self.arity in classnames else 'satisfied_generic_header'

    def get_apply_header(self):
        classnames = {0: 'apply_zeroary_header', 1: 'apply_unary_header', 2: 'apply_binary_header'}
        return classnames[self.arity] if self.arity in classnames else 'apply_generic_header'

    def get_instantiation_tpl(self):
        if self._type == 'EFFECT':
            return 'effect_instantiation'
        elif self._type == 'CONSTRAINT':
            return 'constraint_instantiation'
        else:
            raise RuntimeError("Unknown component type")

    def optimize_code(self, code_blocks, arity):
        """  This is a big hack. But works. """
        if self._type == 'EFFECT':
            if arity == 1:
                return [code.replace('relevant[0]', 'v1') for code in code_blocks]
            elif arity == 2:
                return [code.replace('relevant[0]', 'v1').replace('relevant[1]', 'v2') for code in code_blocks]
            else:
                return code_blocks
        else:
            if arity == 1:
                return [code.replace('relevant[0]', 'v1') for code in code_blocks]
            elif arity == 2:
                return [code.replace('relevant[0]', 'v1').replace('relevant[1]', 'v2') for code in code_blocks]
            else:
                return code_blocks
