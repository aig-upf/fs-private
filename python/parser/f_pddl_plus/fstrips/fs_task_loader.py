from ...pddl.pddl_types import *
from ...pddl.predicates import *
from ...pddl.functions import *
from ...pddl.bounds import *
from ...pddl.metrics import *
from ...pddl.actions import Action
from ...pddl.events import Event
from ...pddl.conditions import *
from ...pddl.effects import *
from ...pddl.f_expression import *
from ...pddl.constraints import *
from .visitor import FunctionalStripsVisitor

class UnresolvedVariableError( Exception ) :
    def __init__(self, value ) :
        self.value = value
    def __str__( self ) :
        return repr(self.value)

class UndeclaredVariable( Exception ) :
    def __init__(self, component, value ) :
        self.component = component
        self.value = value
    def __str__( self ) :
        return 'in {} found undeclared variable {}'.format(self.component, repr(self.value))

class FunctionalStripsLoader( FunctionalStripsVisitor ) :

    domain_name = None
    requirements = []
    types = []
    constants = []
    actions = []
    events = []
    processes = []
    functions = []
    predicates = []
    types_table = {}
    constants_table = {}
    objects_table = {}
    predicates_table = {}
    functions_table = {}

    task_name = None
    task_domain_name = None
    init = []
    goal = None
    constraints = None
    constraint_schemata = []
    type_bounds = []
    objects = []
    metric = None

    axioms = []
    negated = False

    current_params = None
    tokenStream = None

    def initialise(self) :
        # Initialise type information

        self.types.append( Type('object') )
        self.types.append( Type('number') )
        self.types.append( Type('int') )
        self.updateTypeTable()

    def doPostProcessing( self ) :
        self.checkForDuplicateTypes()
        self.updateTypeTable()

    def checkForDuplicateTypes( self ) :
        typenames = set()
        for type in self.types :
            if type in typenames :
                raise SystemExit( "Revise type declarations: '{0}' was defined multiple times".format(type.name) )
            typenames.add(type)

    def updateTypeTable( self ) :
        # Update type table
        for type in self.types :
            try :
                t = self.types_table[ type.name ]

            except KeyError :
                self.types_table[ type.name ] = type

        set_supertypes( self.types )

    def visitDomainName( self, ctx ) :
        self.domain_name = ctx.NAME().getText()

    def visitProblemDecl( self, ctx ) :
        self.task_name = ctx.NAME().getText()

    def visitProblemDomain( self, ctx ) :
        self.task_domain_name = ctx.NAME().getText()

    def visitRequireDef( self, ctx ) :
        for req_ctx in ctx.REQUIRE_KEY() :
            self.requirements.append( req_ctx.getText() )

    def visitTypesDef(self, ctx):
        #print("Type Definitions")
        typenames_list = self.visit(ctx.typedNameList())

        for typename, basename in typenames_list :
            self.types.append( Type(typename, basename))
            self.updateTypeTable()


        #print( "Declared types: {0}".format(len(self.types)))
        #print( self.types )


    def visitNameList( self, ctx ) :
        names = [ name.getText() for name in ctx.NAME() ]
        return names

    def visitSimpleNameList( self, ctx ) :
        names = self.visit( ctx.nameList() )
        return [ (name, 'object') for name in names ]

    def visitNameListWithType( self, ctx ) :
        names = [ ctx.NAME().getText() ] + self.visit( ctx.nameList() )
        names = [ (name, ctx.theType.getText()) for name in names ]
        return names

    def visitComplexNameList(self, ctx) :
        simple = self.visit( ctx.nameList() )
        derived = []
        for sub_ctx in ctx.nameListWithType() :
            derived += self.visit( sub_ctx )
        return simple + derived

    def visitPredicatesDef( self, ctx ) :
        self.predicates = []
        for sub_ctx in ctx.atomicFormulaSkeleton() :
            self.predicates.append( self.visit( sub_ctx ) )
        for p in self.predicates :
            if p.name in self.predicates_table :
                raise SystemExit( "Predicate aliasing is not allowed:\n First predicate: {0}\n Second predicate: {1}".format( str(self.predicates_table[p.name]), str(p)) )
            self.predicates_table[p.name] = p
        # MRJ: Built-ins aren't added to the list of predicates - just
        # registered in the table
        sum_pred = Predicate( '@sum', [])
        self.predicates_table[ '@sum' ] = sum_pred
        alldiff_pred = Predicate( '@alldiff', [])
        self.predicates_table[ '@alldiff' ] = alldiff_pred

        #print("Predicates")
        #for p in self.predicates :
        #    print(p)

    def visitAtomicFormulaSkeleton( self, ctx ) :
        pred_name = ctx.predicate().getText()
        arguments = self.visit( ctx.typedVariableList() )
        return Predicate( pred_name, arguments )

    def visitSimpleVariableList( self, ctx ) :
        var_names = [ TypedObject( name.getText(), 'object' ) for name in ctx.VARIABLE() ]
        return var_names

    def visitComplexVariableList( self, ctx ) :
        untyped_var_names = [ TypedObject( name.getText(), 'object' ) for name in ctx.VARIABLE() ]
        typed_var_names = []
        for sub_ctx in ctx.variableListWithType() :
            typed_var_names += self.visit( sub_ctx )
        return typed_var_names + untyped_var_names

    def visitVariableListWithType( self, ctx ) :
        typed_variable_list = [ ]
        for var_name in ctx.VARIABLE() :
            var_ref = TypedObject( var_name.getText(), ctx.primType().getText() )
            typed_variable_list.append(var_ref)
        return typed_variable_list

    def visitFunctionsDef( self, ctx ) :
        for sub_ctx in ctx.functionDeclGroup() :
            self.functions += self.visit( sub_ctx)
        for f in self.functions :
            if f.name in self.functions_table :
                raise SystemExit( "Function aliasing is not allowed:\n First function: {0} \n Second function: {1}".format(str(self.functions_table[f.name]), str(f)) )
            # Check type of function
            try :
                foo = self.types_table[f.type]
            except KeyError :
                raise SystemExit( "Function {} return type {} is not declared".format(f, f.type) )

            self.functions_table[f.name] = f
        #print('Functions')
        #for f in self.functions :
        #    print(f)

    def visitFunctionDeclGroup( self, ctx ) :
        return_type = ctx.primType().getText()
        functions = []
        for sub_ctx in ctx.atomicFunctionSkeleton() :
            f_name, f_args = self.visit( sub_ctx )
            functions.append( TypedFunction( f_name, f_args, return_type ) )
        return functions

    def visitAtomicFunctionSkeleton( self, ctx ) :
        func_name = ctx.functionSymbol().getText()
        func_args = self.visit( ctx.typedVariableList() )
        return func_name, func_args

    def visitBoundsDecl( self, ctx ) :

        for sub_ctx in ctx.typeBoundsDefinition() :
            self.type_bounds.append( self.visit( sub_ctx ) )
        #print ('Bounds')
        #for b in self.type_bounds :
        #    print(b)

    def visitTypeBoundsDefinition( self, ctx ) :

        b = DomainBound( ctx.NAME().getText(), '{0}[{1}..{2}]'.format( ctx.numericBuiltinType().getText(), ctx.NUMBER(0), ctx.NUMBER(1)  ) )
        return b

    def visitConstantsDef( self, ctx ) :
        constants_definition = self.visit( ctx.typedNameList() )
        self.constants = [ TypedObject(name, objType) for name, objType in constants_definition ]
        for c in self.constants :
            if c.name in self.objects_table :
                raise SystemExit( "Duplicate object found:\n {0} vs. {1}".format(str(c), str(self.objects_table[c.name])))
            self.objects_table[ c.name ] = c
        undef_obj = TypedObject( 'undefined', 'object')
        self.constants.append( undef_obj )
        self.objects_table[ undef_obj.name ] = undef_obj
        #print( "Constants:" )
        #for c in constants :
        #    print(c)

    def visitObjectDecl( self, ctx ) :
        objects_definition = self.visit( ctx.typedNameList() )
        self.objects = [ TypedObject(name, objType ) for name, objType in objects_definition ]
        for o in self.objects :
            if o.name in self.objects_table :
                raise SystemExit( "Duplicate object found:\n {0} vs. {1}".format(str(o), str(self.objects_table[o.name])))
            self.objects_table[ o.name ] = o
        #print( "Objects:")
        #for o in self.objects :
        #    print(o)

    def visitActionDef( self, ctx ) :
        name = ctx.actionSymbol().getText()
        params = self.visit( ctx.typedVariableList() )
        self.current_params = params
        try :
            precondition, effect = self.visit( ctx.actionDefBody() )
        except UndeclaredVariable as error :
            raise SystemExit("Parsing action {}: undeclared variable {}".format(name, error))
        self.current_params = None
        action = Action( name, params, len(params), precondition, effect, None )
        self.actions.append(action)
        #print( 'Action: {0}'.format(name) )
        #print( 'Parameters: {0}'.format(len(params)))
        #for parm in params :
        #    print(parm)
        #precondition.dump()
        #effect.dump()

    def visitEventDef( self, ctx ) :
        name = ctx.eventSymbol().getText()
        params = self.visit( ctx.typedVariableList() )
        self.current_params = params
        try :
            precondition, effect = self.visit( ctx.actionDefBody() )
        except UndeclaredVariable as error :
            raise SystemExit("Parsing event {}: undeclared variable {}".format(name, error))
        self.current_params = None
        evt = Event( name, params, len(params), precondition, effect )
        self.events.append(evt)
        #print( 'Action: {0}'.format(name) )
        #print( 'Parameters: {0}'.format(len(params)))
        #for parm in params :
        #    print(parm)
        #precondition.dump()
        #effect.dump()

    def visitConstraintDef( self, ctx ) :
        name = ctx.constraintSymbol().getText()
        params = self.visit( ctx.typedVariableList() )
        self.current_params = params
        self.negated = False

        try :
            conditions = self.visit( ctx.goalDesc() )
        except UndeclaredVariable as error :
            raise SystemExit("Parsing process {}: undeclared variable in {} found undeclared variable {}".format('state constraint', repr(error)))

        self.current_params = None
        state_constraint = Constraint( name, params, conditions )
        self.constraint_schemata.append( state_constraint )

    def visitActionDefBody( self, ctx) :
        try :
            prec = self.visit( ctx.precondition() )
        except UnresolvedVariableError as e:
            raise UndeclaredVariable('precondition',str(e))
        try :
            unnorm_eff= self.visit( ctx.effect() )
        except UnresolvedVariableError as e :
            raise UndeclaredVariable('effect',str(e))
        norm_eff = unnorm_eff.normalize()
        norm_eff_list = []
        add_effect( norm_eff, norm_eff_list )

        return prec, norm_eff_list

    def visitProcessDef( self, ctx ) :
        name = ctx.actionSymbol().getText()
        params = self.visit( ctx.typedVariableList() )
        self.current_params = params
        try :
            precondition, effect = self.visit( ctx.processDefBody() )
        except UndeclaredVariable as error :
            raise SystemExit("Parsing process {}: undeclared variable {}".format(name, error))
        self.current_params = None
        process = Action( name, params, len(params), precondition, effect, None )
        self.processes.append(process)
        #print( 'Action: {0}'.format(name) )
        #print( 'Parameters: {0}'.format(len(params)))
        #for parm in params :
        #    print(parm)
        #precondition.dump()
        #effect.dump()

    def visitProcessDefBody( self, ctx) :
        try :
            prec = self.visit( ctx.precondition() )
        except UnresolvedVariableError as e:
            raise UndeclaredVariable('precondition',str(e))
        try :
            unnorm_eff= self.visit( ctx.processEffectList() )
        except UnresolvedVariableError as e :
            raise UndeclaredVariable('effect',str(e))
        norm_eff = unnorm_eff.normalize()
        norm_eff_list = []
        add_effect( norm_eff, norm_eff_list )

        return prec, norm_eff_list

    def visitTrivialPrecondition( self, ctx ) :
        return  Truth() # Precondition is true

    def visitRegularPrecondition( self, ctx ) :
        self.negated = False
        return self.visit( ctx.goalDesc() )

    def visitAtomicTermFormula( self, ctx ) :
        atom_name = ctx.predicate().getText()
        arity = 0
        term_type = None
        if atom_name in self.predicates_table :
            arity = len(self.predicates_table[atom_name].arguments)
            term_type = "Predicate"
        elif atom_name in self.functions_table :
            arity = len(self.functions_table[atom_name].arguments)
            term_type = "Function"
        else :
            raise SystemExit( "Predicate/Function '{0}' first seen in atomic formula".format(atom_name) )
        term_list = []
        for term_ctx in ctx.term() :
            term_list.append( self.visit( term_ctx ) )
        if len(term_list) != arity :
            raise SystemExit( "{0} '{1}' used with arity {2}, declared with arity {3}".format(term_type, atom_name, len(term_list), arity))
        return atom_name, term_list

    def visitTermGoalDesc( self, ctx ) :
        atom_name, term_list = self.visit( ctx.atomicTermFormula() )
        if self.negated :
            return NegatedAtom( atom_name, term_list )
        return Atom( atom_name, term_list )

    def visitTermObject( self, ctx ) :
        object_name = ctx.NAME().getText()
        if object_name not in self.objects_table :
            sourceInterval = ctx.getSourceInterval()
            firstToken = self.tokenStream.get(sourceInterval[0])
            line = firstToken.line
            raise SystemExit( "Undeclared object '{}' used as a term in atomic formula\nLine: {} Text: {}".format(object_name, firstToken.line, self.tokenStream.getText(sourceInterval)))
        return self.objects_table[object_name]

    def visitTermNumber( self, ctx ) :
        object_name = ctx.NUMBER().getText()
        try :
            return NumericConstant( int(object_name ) )
        except ValueError :
            return NumericConstant( float(object_name) )

    def visitTermVariable( self, ctx ) :
        variable_name = ctx.VARIABLE().getText()
        if self.current_params is None : return variable_name
        for var_obj in self.current_params :
            if var_obj.name == variable_name :
                return var_obj
        raise UnresolvedVariableError( variable_name )

    def visitGenericFunctionTerm( self, ctx ) :
        func_name = ctx.functionSymbol().getText()
        if func_name not in self.functions_table :
            sourceInterval = ctx.getSourceInterval()
            firstToken = self.tokenStream.get(sourceInterval[0])
            line = firstToken.line
            raise SystemExit( "Function {} first seen used as a term in an atomic formula\n Line: {} Text: '{}'".format(func_name,line, self.tokenStream.getText(sourceInterval)) )
        func_definition = self.functions_table[func_name]
        term_list = []
        for term_ctx in ctx.term() :
            term_list.append( self.visit( term_ctx ) )
        if len(term_list) != len(func_definition.arguments) :
            raise SystemExit( "Line {0}: Function/Predicate {1} used with arity {2}, declared with arity {3}".format(ctx.functionSymbol().start.line,func_name,len(term_list),len(func_definition.arguments)) )
        return FunctionalTerm( func_name, term_list )

    def visitBinaryArithmeticFunctionTerm( self, ctx ) :
        func_name = ctx.binaryOp().getText()
        if func_name not in built_in_functional_symbols :
            raise SystemExit( "Function {0} first seen used as a term in an atomic formula".format(func_name) )
        term_list = []
        for term_ctx in ctx.term() :
            term_list.append( self.visit( term_ctx ) )
        return FunctionalTerm( func_name, term_list )

    def visitUnaryArithmeticFunctionTerm( self, ctx ) :
        func_name = ctx.unaryBuiltIn().getText()
        if func_name not in built_in_functional_symbols :
            raise SystemExit( "Function {0} first seen used as a term in an atomic formula".format(func_name) )
        if func_name == '-' :
            return FunctionalTerm( '*', [ self.visit(ctx.term()), NumericConstant(-1) ] )
        return FunctionalTerm( func_name, [ self.visit(ctx.term()) ])

    def visitAndGoalDesc( self, ctx ) :
        parts = []
        for sub_ctx in ctx.goalDesc() :
            parts.append( self.visit( sub_ctx ) )
        if self.negated :
            return Disjunction( parts )
        return Conjunction( parts )

    def visitOrGoalDesc( self, ctx ) :
        parts = []
        for sub_ctx in ctx.goalDesc() :

            parts.append( self.visit( sub_ctx ) )
        if self.negated :
            return Conjunction( parts )
        return Disjunction( parts )

    def visitNotGoalDesc( self, ctx ) :
        if self.negated :
            self.negated = False
            result = self.visit( ctx.goalDesc() ) # Simplify double negation
            self.negated = True
            return result
        self.negated = True
        result =  self.visit( ctx.goalDesc() ) # propagate negation down
        self.negated = False
        return result

    def visitImplyGoalDesc( self, ctx ) :
        lhs = ctx.goalDesc(0)
        rhs = ctx.goalDesc(1)

        if self.negated :
            self.negated = False
            lhs = self.visit( lhs )
            self.negated = True
            rhs = self.visit( rhs )
        else :
            self.negated = True
            lhs = self.visit( lhs )
            self.negated = False
            rhs = self.visit( rhs )

        if self.negated :
            return Conjunction( [ lhs, rhs ])
        return Disjunction( [lhs, rhs ] )

    def visitExistentialGoalDesc( self, ctx ) :
        params = self.visit( ctx.typedVariableList() )
        self.current_params += params
        formula = self.visit( ctx.goalDesc() )
        if hasattr(ctx, 'negated') and self.negated :
            return UniversalCondition( params, formula )
        return ExistentialCondition( params, formula )

    def visitUniversalGoalDesc( self, ctx ) :
        params = self.visit( ctx.typedVariableList() )
        self.current_params += params
        formula = self.visit( ctx.goalDesc() )
        if type(formula) not in (Conjunction,ConstantCondition,Literal) :
            raise SystemExit("Error: Quantified non-conjunctive formulae are not supported: formula: {}".format(str(formula)))
        if hasattr(ctx, 'negated') and ctx.negated :
            return ExistentialCondition( params, formula )
        return UniversalCondition( params, formula )

    def visitComparisonGoalDesc( self, ctx ) :
        context = ctx.fComp()
        return self.visit( context )

    def visitEquality( self, ctx ) :
        lhs = self.visit(ctx.term(0))
        rhs = self.visit(ctx.term(1))
        if self.negated :
            return NegatedAtom( '=', [lhs, rhs])
        return Atom( '=', [lhs, rhs])

    def visitFComp( self, ctx ) :
        op = ctx.binaryComp().getText()
        neg_op = { '<' : '>=', '>' : '<=', '<=' : '>', '>=' : '<' }
        lhs = self.visit( ctx.fExp(0) )
        rhs = self.visit( ctx.fExp(1) )
        if self.negated :
            if op == '=' :
                return NegatedAtom( op, [lhs,rhs])
            return Atom( neg_op[op], [lhs, rhs] )
        return Atom( op, [lhs, rhs] )

    def visitNumericConstantExpr( self, ctx ) :
        try :
            return NumericConstant( int(ctx.NUMBER().getText()) )
        except ValueError :
            return NumericConstant( float(ctx.NUMBER().getText()) )

    def visitBinaryOperationExpr( self, ctx ) :
        op = ctx.binaryOp().getText()
        lhs = self.visit( ctx.fExp(0))
        rhs = self.visit( ctx.fExp(1))
        return FunctionalTerm( op, [lhs, rhs] )

    def visitUnaryOperationExpr( self, ctx ) :
        op = '*'
        lhs = self.visit(ctx.fExp() )
        rhs = NumericConstant( -1 )
        return PrimitiveNumericExpression( op, [lhs, rhs] )

    def visitFunctionExpr( self, ctx ) :
        return self.visit( ctx.functionTerm() )

    def visitVariableExpr( self, ctx ) :
        variable_name = ctx.VARIABLE().getText()
        if self.current_params is None : return variable_name
        for var_obj in self.current_params :
            if var_obj.name == variable_name :
                return var_obj
        raise UnresolvedVariableError( variable_name )


    def visitGoal( self, ctx ) :
        self.negated = False # at the root, there's no negation
        self.goal = self.visit( ctx.goalDesc() )
        #print("Goal")
        #self.goal.dump()

    def visitSingleEffect( self, ctx ) :
        return ConjunctiveEffect([ self.visit( ctx.cEffect() ) ])

    def visitConjunctiveEffectFormula( self, ctx ) :
        effects = []
        for sub_ctx in ctx.cEffect() :
                effects.append( self.visit( sub_ctx ) )
        return ConjunctiveEffect( effects )
        #try :
        #    for sub_ctx in ctx.processEffect() :
        #        effects.append( self.visit( sub_ctx ) )
        #except AttributeError :
        #    for sub_ctx in ctx.cEffect() :
        #        effects.append( self.visit( sub_ctx ) )
        #return ConjunctiveEffect( effects )

    def visitProcessSingleEffect( self, ctx ) :
        return ConjunctiveEffect([ self.visit( ctx.processEffect() ) ])

    def visitProcessConjunctiveEffectFormula( self, ctx ) :
        effects = []
        for sub_ctx in ctx.processEffect() :
            effects.append( self.visit( sub_ctx ) )
        return ConjunctiveEffect( effects )
        #return self.visitConjunctiveEffectFormula( ctx )

    def visitUniversallyQuantifiedEffect( self, ctx ) :
        scope = self.visit( ctx.typedVariableList() )
        effect_formula = self.visit( ctx.effect() )
        return UniversalEffect( scope, effect_formula )

    def visitConditionalEffect( self, ctx ) :
        self.negated = False # at the root, there's no negation
        condition = self.visit( ctx.goalDesc() )

        effect_formula = self.visit( ctx.condEffect() )
        return ConditionalEffect( condition, effect_formula )

    def visitSimpleEffect( self, ctx ) :
        return ConjunctiveEffect([self.visit( ctx.pEffect() )])

    def visitConjConditionalEffectFormula( self, ctx ) :
        effects = []
        for sub_ctx in ctx.pEffect() :
            effects.append( self.visit(sub_ctx) )
        return ConjunctiveEffect( effects )

    def visitSingleConditionalEffect( self, ctx ) :
        return self.visit( ctx.pEffect() )

    def visitAssignEffect( self, ctx ) :
        operation = ctx.assignOp().getText()
        lhs = self.visit( ctx.functionTerm() )
        rhs = self.visit( ctx.fExp() )
        if operation == 'assign' :
            return AssignmentEffect( lhs, rhs )
        trans_op = { 'scale-up' : '*', 'scale-down' : '/', 'increase' : '+', 'decrease' : '-' }
        #print("{} {} {}".format( trans_op[operation], lhs, rhs))
        new_rhs = FunctionalTerm( trans_op[operation], [lhs, rhs] )
        return AssignmentEffect( lhs, new_rhs ) # This effectively normalizes effects

    def visitProcessAssignEffect( self, ctx ) :
        operation = ctx.processEffectOp().getText()
        lhs = self.visit( ctx.functionTerm() )
        rhs = self.visit( ctx.processEffectExp() )
        if operation in ['assign', 'scale-up', 'scale-down' ] :
            raise SystemExit("Assign/scale up/scale down effects not allowed in processes")
        trans_op = { 'increase' : '+', 'decrease' : '-' }
        new_rhs = FunctionalTerm( trans_op[operation], [lhs, rhs] )
        return AssignmentEffect( lhs, new_rhs ) # This effectively normalizes effects

    def visitFunctionalProcessEffectExpr( self, ctx ) :
        return self.visit( ctx.processFunctionEff())

    def visitConstProcessEffectExpr( self, ctx ) :
        return self.visit( ctx.processConstEff() )

    def visitVariableProcessEffectExpr( self, ctx ) :
        return self.visit( ctx.processVarEff() )

    def visitProcessFunctionEff( self, ctx ) :
        return self.visit( ctx.functionTerm() )

    def visitProcessConstEff( self, ctx ) :
        try :
            return NumericConstant( int(ctx.NUMBER().getText()) )
        except ValueError :
            return NumericConstant( float(ctx.NUMBER().getText()) )

    def visitProcessVarEff( self, ctx ) :
        variable_name = ctx.VARIABLE().getText()
        if self.current_params is None : return variable_name
        for var_obj in self.current_params :
            if var_obj.name == variable_name :
                return var_obj
        raise UnresolvedVariableError( variable_name )

    def visitAssertNegationEffect( self, ctx ) :
        atom_name, term_list = self.visit( ctx.atomicTermFormula() )
        return SimpleEffect( NegatedAtom( atom_name, term_list ) )

    def visitAssertEffect( self, ctx ) :
        atom_name, term_list = self.visit( ctx.atomicTermFormula() )
        return SimpleEffect( Atom( atom_name, term_list ) )

    def visitAssignConstant( self, ctx ) :
        lhs = self.visit( ctx.functionTerm() )
        rhs = self.visit( ctx.term() )
        return AssignmentEffect( lhs, rhs )

    def visitAssignUndefined( self, ctx ) :
        lhs = self.visit( ctx.functionTerm() )
        rhs = self.objects_table['undefined']
        return AssignmentEffect( lhs, rhs )


    def visitInit( self, ctx ) :
        statements = []
        for element_ctx in ctx.initEl() :
            statements.append( self.visit( element_ctx ) )
        false_lits = set()
        true_lits = set()
        assignments = set()
        for stmt in statements :
            if isinstance( stmt, NegatedAtom ) :
                if stmt.predicate in true_lits :
                    raise SystemExit( "Inconsistent initial state: {0} is true and false".format( str(stmt.predicate) ))
                false_lits.add(stmt.predicate)
            elif isinstance( stmt, Atom ) :
                if stmt.predicate in false_lits :
                    raise SystemExit( "Inconsistent initial state: {0} is true and false".format( str(stmt.predicate) ))
                true_lits.add(stmt.predicate)
            else :
                if stmt.fluent in assignments :
                    raise SystemExit("Inconsistent initial state: {0} is assigned two different values".format( str(stmt.fluent) ))
                assignments.add( stmt.fluent )

        for stmt in statements :
            if isinstance( stmt, NegatedAtom ) : continue
            self.init.append( stmt )
        #print( "Init")
        #for e in self.init :
        #    print(e)

    def visitInitLiteral( self, ctx ) :
        return self.visit( ctx.nameLiteral() )

    def visitGroundAtomicFormula( self, ctx ) :
        atom_name = ctx.predicate().getText()
        arity = 0
        term_type = None
        if atom_name in self.predicates_table :
            arity = len(self.predicates_table[atom_name].arguments)
            term_type = "Predicate"
        elif atom_name in self.functions_table :
            arity = len(self.functions_table[atom_name].arguments)
            term_type = "Function"
        else :
            raise SystemExit( "Predicate/Function '{0}' first seen in atomic formula".format(atom_name) )
        term_list = []
        for term_ctx in ctx.groundTerm() :
            term_list.append( self.visit( term_ctx ) )
        if len(term_list) != arity :
            raise SystemExit( "{0} '{1}' used with arity {2}, declared with arity {3}".format(term_type, atom_name, len(term_list), arity))
        return atom_name, term_list

    def visitInitPositiveLiteral( self, ctx ) :
        atom_name, term_list = self.visit( ctx.groundAtomicFormula() )
        return Atom( atom_name, term_list )

    def visitInitNegativeLiteral( self, ctx ) :
        atom_name, term_list = self.visit( ctx.groundAtomicFormula() )
        return NegatedAtom( atom_name, term_list )

    def visitGroundFunctionTerm( self, ctx ) :
        func_name = ctx.functionSymbol().getText()
        if func_name not in self.functions_table :
            raise SystemExit( "Function {0} first seen used as a term in Initial State".format(func_name) )
        term_list = []
        for term_ctx in ctx.groundTerm() :
            term_list.append( self.visit(term_ctx) )
        return FunctionalTerm( func_name, term_list )

    def visitGroundTermObject( self, ctx ) :
        object_name = ctx.NAME().getText()
        if object_name not in self.objects_table :
            raise SystemExit( "Undeclared object {0} used as a term in atomic formula".format(object_name) )
        return self.objects_table[object_name]

    def visitGroundTermNumber( self, ctx ) :
        object_name = ctx.NUMBER().getText()
        try :
            return NumericConstant( int(object_name) )
        except ValueError :
            return NumericConstant( float(object_name) )


    def visitInitAssignmentNumeric( self, ctx ) :
        lhs = self.visit( ctx.groundFunctionTerm() )
        try :
            rhs = NumericConstant( int(ctx.NUMBER().getText()) )
        except ValueError :
            rhs = NumericConstant( float(ctx.NUMBER().getText()) )
        return Assign( lhs, rhs )

    def visitInitAssignmentObject( self, ctx ) :
        lhs = self.visit( ctx.groundFunctionTerm() )
        obj_name = ctx.NAME().getText()
        if not obj_name in self.objects_table :
            raise SystemExit( "Object {0} first seen assigning a value to {1} in the initial state".format( obj_name, str(lhs)) )
        rhs = self.objects_table[ obj_name ]
        return Assign( lhs, rhs )

    def visitExtensionalConstraintGD( self, ctx ) :
        arg_list = []
        for fn_ctx in ctx.groundFunctionTerm() :
            arg_list.append( self.visit(fn_ctx) )
        return  [ Atom( ctx.EXTNAME().getText(), arg_list ) ]

    def visitAlternativeAlwaysConstraint( self, ctx ) :
        return [ self.visit( ctx.goalDesc() ) ]

    def visitConjunctionOfConstraints( self, ctx ) :
        constraints = []
        for conGD_ctx in ctx.prefConGD() :
            constraints += self.visit( conGD_ctx )
        return constraints

    def visitPlainConstraintList( self, ctx ) :
        constraints = []
        for conGD_ctx in ctx.conGD() :
            constraints += self.visit( conGD_ctx )
        return constraints

    def visitProbConstraints( self, ctx ) :
        self.constraints = Conjunction( self.visit( ctx.prefConGD() ) )

    def visitProblemMetric( self, ctx ) :
        optimization = ctx.optimization().getText()
        self.metric = Metric(optimization)
        self.metric.expr = self.visit( ctx.metricFExp() )

    def visitFunctionalExprMetric( self, ctx ) :
        return (None, self.visit( ctx.functionTerm() ))

    def visitCompositeMetric(self, ctx) :
        return (self.visit(ctx.terminalCost()), self.visit(ctx.stageCost()))

    def visitTerminalCost( self, ctx ) :
        return self.visit( ctx.functionTerm() )

    def visitStageCost( self, ctx ) :
        return self.visit( ctx.functionTerm() )

    def visitTotalTimeMetric( self, ctx ) :
        raise SystemExit("Unsupported feature: Minimize total-time metric is not supported")

    def visitIsViolatedMetric( self, ctx ) :
        raise SystemExit("Unsupported feature: Count of violated constraints metric is not supported")
