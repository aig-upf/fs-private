# Generated from java-escape by ANTLR 4.5
from antlr4 import *

# This class defines a complete generic visitor for a parse tree produced by FunctionalStripsParser.

class FunctionalStripsVisitor(ParseTreeVisitor):

    # Visit a parse tree produced by FunctionalStripsParser#pddlDoc.
    def visitPddlDoc(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#domain.
    def visitDomain(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#free_functionsDef.
    def visitFree_functionsDef(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#domainName.
    def visitDomainName(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#requireDef.
    def visitRequireDef(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#typesDef.
    def visitTypesDef(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#Integer.
    def visitInteger(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#Float.
    def visitFloat(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#Number.
    def visitNumber(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#NumericBuiltin.
    def visitNumericBuiltin(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#ObjectBuiltin.
    def visitObjectBuiltin(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#nameList.
    def visitNameList(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#SimpleNameList.
    def visitSimpleNameList(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#ComplexNameList.
    def visitComplexNameList(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#nameListWithType.
    def visitNameListWithType(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#typename.
    def visitTypename(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#primType.
    def visitPrimType(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#functionsDef.
    def visitFunctionsDef(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#functionDeclGroup.
    def visitFunctionDeclGroup(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#atomicFunctionSkeleton.
    def visitAtomicFunctionSkeleton(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#functionSymbol.
    def visitFunctionSymbol(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#constantsDef.
    def visitConstantsDef(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#predicatesDef.
    def visitPredicatesDef(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#atomicFormulaSkeleton.
    def visitAtomicFormulaSkeleton(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#predicate.
    def visitPredicate(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#SimpleVariableList.
    def visitSimpleVariableList(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#ComplexVariableList.
    def visitComplexVariableList(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#variableListWithType.
    def visitVariableListWithType(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#structureDef.
    def visitStructureDef(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#actionDef.
    def visitActionDef(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#constraintDef.
    def visitConstraintDef(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#eventDef.
    def visitEventDef(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#actionSymbol.
    def visitActionSymbol(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#constraintSymbol.
    def visitConstraintSymbol(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#eventSymbol.
    def visitEventSymbol(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#actionDefBody.
    def visitActionDefBody(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#TrivialPrecondition.
    def visitTrivialPrecondition(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#RegularPrecondition.
    def visitRegularPrecondition(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#TermGoalDesc.
    def visitTermGoalDesc(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#AndGoalDesc.
    def visitAndGoalDesc(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#OrGoalDesc.
    def visitOrGoalDesc(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#NotGoalDesc.
    def visitNotGoalDesc(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#ImplyGoalDesc.
    def visitImplyGoalDesc(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#ExistentialGoalDesc.
    def visitExistentialGoalDesc(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#UniversalGoalDesc.
    def visitUniversalGoalDesc(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#ComparisonGoalDesc.
    def visitComparisonGoalDesc(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#EqualityGoalDesc.
    def visitEqualityGoalDesc(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#equality.
    def visitEquality(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#fComp.
    def visitFComp(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#atomicTermFormula.
    def visitAtomicTermFormula(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#TermObject.
    def visitTermObject(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#TermNumber.
    def visitTermNumber(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#TermVariable.
    def visitTermVariable(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#TermTimeStep.
    def visitTermTimeStep(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#TermFunction.
    def visitTermFunction(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#GenericFunctionTerm.
    def visitGenericFunctionTerm(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#BinaryArithmeticFunctionTerm.
    def visitBinaryArithmeticFunctionTerm(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#UnaryArithmeticFunctionTerm.
    def visitUnaryArithmeticFunctionTerm(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#processDef.
    def visitProcessDef(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#processDefBody.
    def visitProcessDefBody(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#ProcessConjunctiveEffectFormula.
    def visitProcessConjunctiveEffectFormula(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#ProcessSingleEffect.
    def visitProcessSingleEffect(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#ProcessAssignEffect.
    def visitProcessAssignEffect(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#durativeActionDef.
    def visitDurativeActionDef(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#daDefBody.
    def visitDaDefBody(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#daGD.
    def visitDaGD(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#prefTimedGD.
    def visitPrefTimedGD(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#timedGD.
    def visitTimedGD(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#timeSpecifier.
    def visitTimeSpecifier(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#interval.
    def visitInterval(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#derivedDef.
    def visitDerivedDef(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#FunctionExpr.
    def visitFunctionExpr(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#NumericConstantExpr.
    def visitNumericConstantExpr(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#VariableExpr.
    def visitVariableExpr(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#FunctionalProcessEffectExpr.
    def visitFunctionalProcessEffectExpr(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#ConstProcessEffectExpr.
    def visitConstProcessEffectExpr(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#VariableProcessEffectExpr.
    def visitVariableProcessEffectExpr(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#processFunctionEff.
    def visitProcessFunctionEff(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#processConstEff.
    def visitProcessConstEff(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#processVarEff.
    def visitProcessVarEff(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#fHead.
    def visitFHead(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#ConjunctiveEffectFormula.
    def visitConjunctiveEffectFormula(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#SingleEffect.
    def visitSingleEffect(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#UniversallyQuantifiedEffect.
    def visitUniversallyQuantifiedEffect(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#ConditionalEffect.
    def visitConditionalEffect(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#SimpleEffect.
    def visitSimpleEffect(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#AssignEffect.
    def visitAssignEffect(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#AssertNegationEffect.
    def visitAssertNegationEffect(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#AssertEffect.
    def visitAssertEffect(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#AssignConstant.
    def visitAssignConstant(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#AssignUndefined.
    def visitAssignUndefined(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#ConjConditionalEffectFormula.
    def visitConjConditionalEffectFormula(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#SingleConditionalEffect.
    def visitSingleConditionalEffect(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#binaryOp.
    def visitBinaryOp(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#unaryBuiltIn.
    def visitUnaryBuiltIn(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#multiOp.
    def visitMultiOp(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#binaryComp.
    def visitBinaryComp(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#assignOp.
    def visitAssignOp(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#processEffectOp.
    def visitProcessEffectOp(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#durationConstraint.
    def visitDurationConstraint(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#simpleDurationConstraint.
    def visitSimpleDurationConstraint(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#durOp.
    def visitDurOp(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#durValue.
    def visitDurValue(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#daEffect.
    def visitDaEffect(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#timedEffect.
    def visitTimedEffect(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#fAssignDA.
    def visitFAssignDA(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#fExpDA.
    def visitFExpDA(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#problem.
    def visitProblem(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#problemMeta.
    def visitProblemMeta(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#problemDecl.
    def visitProblemDecl(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#problemDomain.
    def visitProblemDomain(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#objectDecl.
    def visitObjectDecl(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#boundsDecl.
    def visitBoundsDecl(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#typeBoundsDefinition.
    def visitTypeBoundsDefinition(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#init.
    def visitInit(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#GroundTermObject.
    def visitGroundTermObject(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#GroundTermNumber.
    def visitGroundTermNumber(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#GroundTermFunction.
    def visitGroundTermFunction(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#groundFunctionTerm.
    def visitGroundFunctionTerm(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#InitLiteral.
    def visitInitLiteral(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#InitAssignmentNumeric.
    def visitInitAssignmentNumeric(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#InitTimedLiteral.
    def visitInitTimedLiteral(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#InitAssignmentObject.
    def visitInitAssignmentObject(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#InitPositiveLiteral.
    def visitInitPositiveLiteral(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#InitNegativeLiteral.
    def visitInitNegativeLiteral(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#groundAtomicFormula.
    def visitGroundAtomicFormula(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#goal.
    def visitGoal(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#probConstraints.
    def visitProbConstraints(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#ConjunctionOfConstraints.
    def visitConjunctionOfConstraints(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#UniversallyQuantifiedConstraint.
    def visitUniversallyQuantifiedConstraint(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#PreferenceConstraint.
    def visitPreferenceConstraint(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#PlainConstraintList.
    def visitPlainConstraintList(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#ProblemMetric.
    def visitProblemMetric(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#optimization.
    def visitOptimization(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#FunctionalExpr.
    def visitFunctionalExpr(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#TotalTimeMetric.
    def visitTotalTimeMetric(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#IsViolatedMetric.
    def visitIsViolatedMetric(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#ConjunctiveConstraint.
    def visitConjunctiveConstraint(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#ForallConstraint.
    def visitForallConstraint(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#AtEndConstraint.
    def visitAtEndConstraint(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#AlwaysConstraint.
    def visitAlwaysConstraint(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#SometimeConstraint.
    def visitSometimeConstraint(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#WithinConstraint.
    def visitWithinConstraint(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#AtMostOnceConstraint.
    def visitAtMostOnceConstraint(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#SometimeAfterConstraint.
    def visitSometimeAfterConstraint(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#SometimeBeforeConstraint.
    def visitSometimeBeforeConstraint(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#AlwaysWithinConstraint.
    def visitAlwaysWithinConstraint(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#HoldDuringConstraint.
    def visitHoldDuringConstraint(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#HoldAfterConstraint.
    def visitHoldAfterConstraint(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#ExtensionalConstraintGD.
    def visitExtensionalConstraintGD(self, ctx):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by FunctionalStripsParser#AlternativeAlwaysConstraint.
    def visitAlternativeAlwaysConstraint(self, ctx):
        return self.visitChildren(ctx)


