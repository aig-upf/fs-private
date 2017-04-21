
/* Pddl 3.1, extended with FS0 specific stuff*/

grammar FunctionalStrips;

tokens {
    DOMAIN,
    DOMAIN_NAME,
    REQUIREMENTS,
    TYPES,
    EITHER_TYPE,
    CONSTANTS,
    FUNCTIONS,
    FREE_FUNCTIONS,
    PREDICATES,
    ACTION,
    CONSTRAINT,
    EVENT,
    GLOBAL_CONSTRAINT,
    DURATIVE_ACTION,
    PROBLEM,
    PROBLEM_NAME,
    PROBLEM_DOMAIN,
    OBJECTS,
    INIT,
    FUNC_HEAD,
    PRECONDITION,
    EFFECT,
    AND_GD,
    OR_GD,
	  NOT_GD,
	  IMPLY_GD,
	  EXISTS_GD,
	  FORALL_GD,
	  COMPARISON_GD,
	  AND_EFFECT,
	  FORALL_EFFECT,
	  WHEN_EFFECT,
	  ASSIGN_EFFECT,
	  NOT_EFFECT,
	  PRED_HEAD,
	  GOAL,
	  BINARY_OP,
	  EQUALITY_CON,
	  MULTI_OP,
	  MINUS_OP,
	  UNARY_MINUS,
	  INIT_EQ,
	  INIT_AT,
	  NOT_PRED_INIT,
	  PRED_INST,
	  PROBLEM_CONSTRAINT,
	  PROBLEM_METRIC
}

/************* Start of grammar *******************/

pddlDoc : domain | problem;

/************* DOMAINS ****************************/

domain
    : '(' 'define' domainName
      requireDef?
      typesDef?
      constantsDef?
      predicatesDef?
      functionsDef?
      free_functionsDef?
      structureDef*
      ')'
    ;

free_functionsDef
	: '(' ':free_functions' functionDeclGroup* ')'
	;

domainName
    : '(' 'domain' NAME ')'
    ;

requireDef
	: '(' ':requirements' REQUIRE_KEY+ ')'
	;

typesDef
	: '(' ':types' typedNameList ')'
	;

numericBuiltinType
  : 'int'             # Integer
  | 'float'           # Float
  | 'number'          # Number
  ;

builtinType
  : numericBuiltinType  # NumericBuiltin
  | 'object'            # ObjectBuiltin
  ;


// If have any typed names, they must come FIRST!
nameList : NAME*;

typedNameList
    : nameList                     # SimpleNameList
    | nameListWithType+ nameList   # ComplexNameList
    ;

nameListWithType
    : NAME nameList '-' theType=typename
	  ;

typename
	: '(' 'either' primType+ ')'
	| primType
	;

primType : NAME | builtinType;

functionsDef
	: '(' ':functions' functionDeclGroup* ')'
	;

// Now functions must have types
//functionDecl
//	: atomicFunctionSkeleton+ '-' functionType
//	;

functionDeclGroup
    : atomicFunctionSkeleton+ '-' primType
    ;

//atomicFunctionSkeleton
//	: '(' functionSymbol functionTypedList ')'
//	;

atomicFunctionSkeleton
    : '(' functionSymbol typedVariableList ')'
    ;

functionSymbol : NAME | EXTNAME;

//functionType :  primType ;

//functionTypedList : VARIABLE+ '-' functionType functionTypedList |
//                    VARIABLE+ |
//                    ;

constantsDef
	: '(' ':constants' typedNameList ')'
	;

predicatesDef
	: '(' ':predicates' atomicFormulaSkeleton* ')'
	;

atomicFormulaSkeleton
	: '(' predicate typedVariableList ')'
	;

predicate : NAME | EXTNAME;

// If have any typed variables, they must come FIRST!
typedVariableList
    : VARIABLE*                               # SimpleVariableList
    | variableListWithType+ VARIABLE*         # ComplexVariableList
    ;

variableListWithType
    : VARIABLE+ '-' primType
    ;

structureDef
	: actionDef
    | eventDef
	| durativeActionDef
	| derivedDef
	| constraintDef
    | processDef
	;


/************* ACTIONS ****************************/

actionDef
	: '(' ':action' actionSymbol
	      ':parameters'  '(' typedVariableList ')'
           actionDefBody ')'
    ;

constraintDef
	: '(' ':constraint' constraintSymbol
	      ':parameters'  '(' typedVariableList ')'
          ':condition' goalDesc ')'
    ;


eventDef
	: '(' ':event' eventSymbol
	      ':parameters'  '(' typedVariableList ')'
           actionDefBody ')'
    ;

actionSymbol : NAME ;

constraintSymbol : NAME ;

eventSymbol : NAME;

// Should allow preGD instead of goalDesc for preconditions -
// but I can't get the LL(*) parsing to work
// This means 'preference' preconditions cannot be used
actionDefBody
  : PREC_TAG precondition EFF_TAG effect
  ;

precondition
  : '(' ')'        # TrivialPrecondition
  | goalDesc       # RegularPrecondition
  ;

goalDesc
	:  atomicTermFormula                                     # TermGoalDesc
	| '(' 'and' goalDesc* ')'                                # AndGoalDesc
	| '(' 'or' goalDesc* ')'                                 # OrGoalDesc
	| '(' 'not' goalDesc ')'                                 # NotGoalDesc
	| '(' 'imply' goalDesc goalDesc ')'                      # ImplyGoalDesc
	| '(' 'exists' '(' typedVariableList ')' goalDesc ')'    # ExistentialGoalDesc
	| '(' 'forall' '(' typedVariableList ')' goalDesc ')'    # UniversalGoalDesc
  | fComp                                                    # ComparisonGoalDesc
	| equality                                               # EqualityGoalDesc
  ;

equality
	: '(' '=' term term ')'
	;
fComp
	: '(' binaryComp fExp fExp ')'
	;

atomicTermFormula
	: '(' predicate term* ')'
	;

term :
  NAME                #TermObject
  | NUMBER            #TermNumber
  | VARIABLE          #TermVariable
  | '#t'              #TermTimeStep
  | functionTerm      #TermFunction
  ;

functionTerm :
  '(' functionSymbol term* ')'        #GenericFunctionTerm
  | '(' binaryOp term term ')'        #BinaryArithmeticFunctionTerm
  | '(' unaryBuiltIn term ')'         #UnaryArithmeticFunctionTerm
  ;

/************* PROCESSES ****************************/

processDef
	: '(' ':process' actionSymbol
	      ':parameters'  '(' typedVariableList ')'
           processDefBody ')'
    ;

processDefBody
  : PREC_TAG precondition EFF_TAG processEffectList
  ;

processEffectList
	: '(' 'and' processEffect* ')'       #ProcessConjunctiveEffectFormula
	| processEffect                      #ProcessSingleEffect
	;

processEffect
	: '(' processEffectOp functionTerm processEffectExp ')' # ProcessAssignEffect
	;

/************* DURATIVE ACTIONS ****************************/

durativeActionDef
	: '(' ':durative-action' actionSymbol
	      ':parameters'  '(' typedVariableList ')'
           daDefBody ')'
    | '(' ':durative-action' actionSymbol
  	      ':parameters'  '(' ')'
             daDefBody ')'
    ;

daDefBody
	: ':duration' durationConstraint
	| ':condition' (('(' ')') | daGD)
  | ':effect' (('(' ')') | daEffect)
  ;

daGD
	: prefTimedGD
	| '(' 'and' daGD* ')'
	| '(' 'forall' '(' typedVariableList ')' daGD ')'
	;

prefTimedGD
	: timedGD
	| '(' 'preference' NAME? timedGD ')'
	;

timedGD
	: '(' 'at' timeSpecifier goalDesc ')'
	| '(' 'over' interval goalDesc ')'
	;

timeSpecifier : 'start' | 'end' ;
interval : 'all' ;

/************* DERIVED DEFINITIONS ****************************/

derivedDef
	: '(' ':derived' typedVariableList goalDesc ')'
	;

/************* EXPRESSIONS ****************************/

fExp
  : functionTerm  #FunctionExpr
  | NUMBER        #NumericConstantExpr
  | VARIABLE      #VariableExpr
  ;

processEffectExp
    : '(' '*' processFunctionEff ')' #FunctionalProcessEffectExpr
    | '(' '*' processConstEff ')'    #ConstProcessEffectExpr
    | '(' '*' processVarEff ')'      #VariableProcessEffectExpr
    ;

processFunctionEff
    : functionTerm '#t'
    | '#t' functionTerm
    ;

processConstEff
    : NUMBER '#t'
    | '#t' NUMBER
    ;

processVarEff
    : VARIABLE '#t'
    | '#t' VARIABLE
    ;


fHead
	: '(' functionSymbol term* ')'
	;

effect
	: '(' 'and' cEffect* ')'       #ConjunctiveEffectFormula
	| cEffect                      #SingleEffect
	;

cEffect
	: '(' 'forall' '(' typedVariableList ')' effect ')'  # UniversallyQuantifiedEffect
	| '(' 'when' goalDesc condEffect ')'                 # ConditionalEffect
	| pEffect                                            # SimpleEffect
	;

pEffect
	: '(' assignOp functionTerm fExp ')'                 # AssignEffect
	| '(' 'not' atomicTermFormula ')'                    # AssertNegationEffect
	| atomicTermFormula                                  # AssertEffect
  | '(' 'assign' functionTerm term ')'                 # AssignConstant
  | '(' 'assign' functionTerm 'undefined' ')'          # AssignUndefined
	;


// TODO: why is this different from the "and cEffect" above? Does it matter?
// ANSWER: Because conditional effects can only be conjunctions of 'simple effects'
condEffect
	: '(' 'and' pEffect* ')'       # ConjConditionalEffectFormula
	| pEffect                      # SingleConditionalEffect
	;

// TODO: should these be uppercase & lexer section?
binaryOp : '*' | '+' | '-' | '/' | '^';

unaryBuiltIn : '-' | 'sin' | 'cos' | 'sqrt' | 'tan' | 'acos' | 'asin' | 'atan';

multiOp	: '*' | '+' ;

binaryComp : '>' | '<' | '=' | '>=' | '<=' ;

assignOp : 'assign' | 'scale-up' | 'scale-down' | 'increase' | 'decrease' ;

processEffectOp : 'increase' | 'decrease';


/************* DURATIONS  ****************************/

durationConstraint
	: '(' 'and' simpleDurationConstraint+ ')'
	| '(' ')'
	| simpleDurationConstraint
	;

simpleDurationConstraint
	: '(' durOp '?duration' durValue ')'
	| '(' 'at' timeSpecifier simpleDurationConstraint ')'
	;

durOp : '<=' | '>=' | '=' ;

durValue : NUMBER | fExp ;

daEffect
	: '(' 'and' daEffect* ')'
	| timedEffect
	| '(' 'forall' '(' typedVariableList ')' daEffect ')'
	| '(' 'when' daGD timedEffect ')'
	| '(' assignOp fHead fExpDA ')'
	;

timedEffect
	: '(' 'at' timeSpecifier daEffect ')'     // BNF has a-effect here, but not defined anywhere
	| '(' 'at' timeSpecifier fAssignDA ')'
	| '(' assignOp fHead fExp ')'         // BNF has assign-op-t and f-exp-t here, but not defined anywhere
	;

fAssignDA
	: '(' assignOp fHead fExpDA ')'
	;

fExpDA
	: '(' ((binaryOp fExpDA fExpDA) | ('-' fExpDA)) ')'
	| '?duration'
	| fExp
	;

/************* PROBLEMS ****************************/

problem
	: '(' 'define' problemDecl
	  problemDomain
      requireDef?
      objectDecl?
      init
      goal
      problemMeta*
      // lengthSpec? This is not defined anywhere in the BNF spec
      ')'
    ;

problemMeta
    : probConstraints
    | boundsDecl
    | metricSpec
    ;

problemDecl
    : '(' 'problem' NAME ')'
    ;

problemDomain
	: '(' ':domain' NAME ')'
	;

objectDecl
	: '(' ':objects' typedNameList ')'
	;


boundsDecl
  : '(' ':bounds' typeBoundsDefinition+ ')'
  ;

typeBoundsDefinition
  : '(' NAME '-' numericBuiltinType '[' NUMBER '..' NUMBER ']' ')'
  ;

init
	: '(' ':init' initEl* ')'
	;


groundTerm :
  NAME                      #GroundTermObject
  | NUMBER                  #GroundTermNumber
  | groundFunctionTerm      #GroundTermFunction
  ;

groundFunctionTerm : '(' functionSymbol groundTerm* ')';

initEl
	: nameLiteral                             #InitLiteral
	| '(' '=' groundFunctionTerm NUMBER ')'   #InitAssignmentNumeric
	| '(' 'at' NUMBER nameLiteral ')'         #InitTimedLiteral
    | '(' '=' groundFunctionTerm NAME ')'     #InitAssignmentObject
	;

nameLiteral
	: groundAtomicFormula                        #InitPositiveLiteral
	| '(' 'not' groundAtomicFormula ')'          #InitNegativeLiteral
	;

groundAtomicFormula
	: '(' predicate groundTerm* ')'
	;

// Should allow preGD instead of goalDesc -
// but I can't get the LL(*) parsing to work
// This means 'preference' preconditions cannot be used

goal : '(' ':goal' goalDesc ')'  ;

probConstraints
	: '(' ':constraints'  prefConGD ')'
	;

prefConGD
	: '(' 'and' prefConGD* ')'                                 # ConjunctionOfConstraints
	| '(' 'forall' '(' typedVariableList ')' prefConGD ')'     # UniversallyQuantifiedConstraint
	| '(' 'preference' NAME? conGD ')'                         # PreferenceConstraint
	| conGD+                                                   # PlainConstraintList
	;

metricSpec
	: '(' ':metric' optimization metricFExp ')' # ProblemMetric
	;

optimization : 'minimize' | 'maximize' ;

metricFExp
	: functionTerm                                 #FunctionalExpr
    | '(total-time)'                               #TotalTimeMetric
	| '(' 'is-violated' NAME ')'                   #IsViolatedMetric
	;

/************* CONSTRAINTS ****************************/

conGD
	: '(' 'and' conGD+ ')'                                 # ConjunctiveConstraint
	| '(' 'forall' '(' typedVariableList ')' conGD ')'     # ForallConstraint
	| '(' 'at' 'end' goalDesc ')'                          # AtEndConstraint
  | '(' 'always' goalDesc ')'                            # AlwaysConstraint
	| '(' 'sometime' goalDesc ')'                          # SometimeConstraint
 	| '(' 'within' NUMBER goalDesc ')'                     # WithinConstraint
	| '(' 'at-most-once' goalDesc ')'                      # AtMostOnceConstraint
	| '(' 'sometime-after' goalDesc goalDesc ')'           # SometimeAfterConstraint
	| '(' 'sometime-before' goalDesc goalDesc ')'          # SometimeBeforeConstraint
	| '(' 'always-within' NUMBER goalDesc goalDesc ')'     # AlwaysWithinConstraint
	| '(' 'hold-during' NUMBER NUMBER goalDesc ')'         # HoldDuringConstraint
	| '(' 'hold-after' NUMBER goalDesc ')'                 # HoldAfterConstraint
  | '(' EXTNAME groundFunctionTerm+ ')'                  # ExtensionalConstraintGD
  | goalDesc                                             # AlternativeAlwaysConstraint
//  | '(' 'sum_constraint' basicFunctionTerm+ ')'        # SumConstraintGD
	;



/************* LEXER ****************************/


REQUIRE_KEY
    : ':strips'
    | ':typing'
    | ':negative-preconditions'
    | ':disjunctive-preconditions'
    | ':equality'
    | ':existential-preconditions'
    | ':universal-preconditions'
    | ':quantified-preconditions'
    | ':conditional-effects'
    | ':object-fluents'
    | ':numeric-fluents'
    | ':fluents'
    | ':adl'
    | ':durative-actions'
    | ':derived-predicates'
    | ':timed-initial-literals'
    | ':preferences'
    | ':constraints'
    ;


NAME:    LETTER VALID_NAME_ANY_CHAR* ;

EXTNAME: '@' LETTER VALID_NAME_ANY_CHAR* ;

fragment LETTER:	'a'..'z' | 'A'..'Z';

fragment ANY_CHAR: LETTER | '0'..'9' | '-' | '_';

fragment VALID_NAME_ANY_CHAR: LETTER | '0'..'9' | '_';

PREC_TAG: ':precondition';
EFF_TAG : ':effect';

VARIABLE : '?' LETTER ANY_CHAR*  ;

NUMBER : ('-')? DIGIT+ ('.' DIGIT+)? ;

fragment DIGIT: '0'..'9';

LINE_COMMENT
    : (';' ~('\n'|'\r')* '\r'? '\n') -> channel(HIDDEN)
    ;

WHITESPACE
    :   (   ' '
        |   '\t'
        |   '\r'
        |   '\n'
        )+ -> channel(HIDDEN)
    ;
