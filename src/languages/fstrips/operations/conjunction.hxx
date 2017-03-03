
#pragma once

#include <utils/visitor.hxx>
#include <languages/fstrips/formulae.hxx>

//
// A somewhat involved decoupled implementation of a generic conjunction
// operation over two formulas.
//
// In order to avoid the use of dynamic_cast's, the implementation follows
// a (double) use of the visitor pattern; with two different types of visitors.
// The first visitor type, BootstrappingConjunctionVisitor, is responsible
// for creating a second 'ConjunctionVisitor' visitor, whose exact type matches that of 
// the LHS of the conjunction operation. 
// This second concrete ConjunctionVisitor is responsible for implementing the final
// conjunction operation, visiting the RHS operator, and taking into account that it already
// knows what the concrete type of the LHS is.
//

namespace fs0 { namespace language { namespace fstrips {
	
/*
An arbitrary ordering among types:
Formula
Tautology
Contradiction
AtomicFormula
Conjunction
ExistentiallyQuantifiedFormula
*/


//! The main operation: a conjunction between two arbitrary formulas.
Formula* conjunction(const Formula& lhs, const Formula& rhs);

class ConjunctionVisitor
    : public Loki::BaseVisitor
    , public Loki::Visitor<Formula, void, true>
    , public Loki::Visitor<Tautology, void, true>
	, public Loki::Visitor<Contradiction, void, true>
	, public Loki::Visitor<AtomicFormula, void, true>
	, public Loki::Visitor<Conjunction, void, true>
	, public Loki::Visitor<ExistentiallyQuantifiedFormula, void, true>
{
public:
	ConjunctionVisitor() : _result(nullptr) {}
	virtual ~ConjunctionVisitor() = default;

	Formula* _result;	
};

class TautologyLhsConjunctionVisitor : public ConjunctionVisitor
{
public:
	TautologyLhsConjunctionVisitor(const Tautology& lhs) {}

	void Visit(const Formula& lhs) override;
	void Visit(const Tautology& lhs) override;
	void Visit(const Contradiction& lhs) override;
	void Visit(const AtomicFormula& lhs) override;
	void Visit(const Conjunction& lhs) override;
	void Visit(const ExistentiallyQuantifiedFormula& lhs) override;
};

class ContradictionLhsConjunction : public ConjunctionVisitor
{
public:
	ContradictionLhsConjunction(const Contradiction& lhs) {}

	void Visit(const Formula& lhs) override;
	void Visit(const Tautology& lhs) override;
	void Visit(const Contradiction& lhs) override;
	void Visit(const AtomicFormula& lhs) override;
	void Visit(const Conjunction& lhs) override;
	void Visit(const ExistentiallyQuantifiedFormula& lhs) override;
};

class AtomicFormulaLhsConjunction : public ConjunctionVisitor
{
	const AtomicFormula& _lhs;
public:
	AtomicFormulaLhsConjunction(const AtomicFormula& lhs) : _lhs(lhs) {}

	void Visit(const Formula& lhs) override;
	void Visit(const Tautology& lhs) override;
	void Visit(const Contradiction& lhs) override;
	void Visit(const AtomicFormula& lhs) override;
	void Visit(const Conjunction& lhs) override;
	void Visit(const ExistentiallyQuantifiedFormula& lhs) override;
};

class ConjunctionLhsConjunction : public ConjunctionVisitor
{
	const Conjunction& _lhs;
public:
	ConjunctionLhsConjunction(const Conjunction& lhs) : _lhs(lhs) {}

	void Visit(const Formula& lhs) override;
	void Visit(const Tautology& lhs) override;
	void Visit(const Contradiction& lhs) override;
	void Visit(const AtomicFormula& lhs) override;
	void Visit(const Conjunction& lhs) override;
	void Visit(const ExistentiallyQuantifiedFormula& lhs) override;
};

class ExistentiallyQuantifiedFormulaLhsConjunction : public ConjunctionVisitor
{
	const ExistentiallyQuantifiedFormula& _lhs;
public:
	ExistentiallyQuantifiedFormulaLhsConjunction(const ExistentiallyQuantifiedFormula& lhs) : _lhs(lhs) {}

	void Visit(const Formula& lhs) override;
	void Visit(const Tautology& lhs) override;
	void Visit(const Contradiction& lhs) override;
	void Visit(const AtomicFormula& lhs) override;
	void Visit(const Conjunction& lhs) override;
	void Visit(const ExistentiallyQuantifiedFormula& lhs) override;
};



class BootstrappingConjunctionVisitor
    : public Loki::BaseVisitor
    , public Loki::Visitor<Formula, void, true>
    , public Loki::Visitor<Tautology, void, true>
	, public Loki::Visitor<Contradiction, void, true>
	, public Loki::Visitor<AtomicFormula, void, true>
	, public Loki::Visitor<Conjunction, void, true>
	, public Loki::Visitor<ExistentiallyQuantifiedFormula, void, true>
{
	using VisitorPT = std::unique_ptr<ConjunctionVisitor>;
	
public:
	BootstrappingConjunctionVisitor() = default;
	~BootstrappingConjunctionVisitor() = default;
	
 	void Visit(const Formula& lhs) override { throw UnimplementedFeatureException(""); }
	void Visit(const Tautology& lhs) override { _visitor = VisitorPT(new TautologyLhsConjunctionVisitor(lhs)); }
	void Visit(const Contradiction& lhs) override { _visitor = VisitorPT(new ContradictionLhsConjunction(lhs)); }
	void Visit(const AtomicFormula& lhs) override { _visitor = VisitorPT(new AtomicFormulaLhsConjunction(lhs)); }
	void Visit(const Conjunction& lhs) override { _visitor = VisitorPT(new ConjunctionLhsConjunction(lhs)); }
	void Visit(const ExistentiallyQuantifiedFormula& lhs) override { _visitor = VisitorPT(new ExistentiallyQuantifiedFormulaLhsConjunction(lhs)); }

	VisitorPT _visitor;	
};


// CURRENTLY UNUSED - TODO - REMOVE
// class OpConjunction {
// 	using DispatcherT = utils::FnDispatcher<Formula, Formula, Formula*, utils::StaticCaster>;
// 	
// public:
// 	
// 	OpConjunction() {
// 		/*
// 		_dispatcher.Add<Contradiction, AtomicFormula, conjunctionunction, true>();
// 		_dispatcher.Add<Contradiction, Conjunction, conjunctionunction, true>();
// 		_dispatcher.Add<Contradiction, ExistentiallyQuantifiedFormula, conjunctionunction, true>();
// 		_dispatcher.Add<Contradiction, Tautology, conjunctionunction, true>();
// 		_dispatcher.Add<Contradiction, Contradiction, conjunctionunction, true>();
// 		
// 		_dispatcher.Add<Tautology, AtomicFormula, conjunctionunction, true>();
// 		_dispatcher.Add<Tautology, Conjunction, conjunctionunction, true>();
// 		_dispatcher.Add<Tautology, ExistentiallyQuantifiedFormula, conjunctionunction, true>();
// 		_dispatcher.Add<Tautology, Tautology, conjunctionunction, true>();
// 		_dispatcher.Add<Tautology, Contradiction, conjunctionunction, true>();		
// 		*/
// 		
// 		
// 		
// 		// Test
// 		Tautology t1, t2;
// 		Contradiction c1, c2;
// 		EQAtomicFormula form({});
// 		
// 		/*
// 		_dispatcher.Go(t1, t2);
// 		_dispatcher.Go(c1, c2);
// 		_dispatcher.Go(c1, form);
// 		*/
// 		
//  		conjunction(c1, form);
// 
// 		throw std::runtime_error("DONE");
// 	}	
// 	
// protected:
// 	DispatcherT _dispatcher;
// };
	
} } } // namespaces

