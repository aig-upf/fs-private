
#pragma once

#include <vector>

#include <utils/visitor.hxx>
#include <languages/fstrips/language_fwd.hxx>


namespace fs0 { class Binding; class ProblemInfo; class object_id; }

namespace fs0 { namespace language { namespace fstrips {


//! Processes a formula possibly containing bound variables and non-consolidated state variables,
//! consolidating all possible state variables and performing the bindings according to the given variable binding
const Formula* bind(const Formula& formula, const Binding& binding, const ProblemInfo& info);

class FormulaBindingVisitor
    : public Loki::BaseVisitor
    , public Loki::Visitor<Formula, void, true>
    , public Loki::Visitor<Tautology, void, true>
	, public Loki::Visitor<Contradiction, void, true>
	, public Loki::Visitor<AtomicFormula, void, true>
	, public Loki::Visitor<Conjunction, void, true>
	, public Loki::Visitor<Disjunction, void, true>
	, public Loki::Visitor<ExistentiallyQuantifiedFormula, void, true>
	, public Loki::Visitor<UniversallyQuantifiedFormula, void, true>
	, public Loki::Visitor<AxiomaticFormula, void, true>

{
private:
	const Binding& _binding;
	const ProblemInfo& _info;

public:
	FormulaBindingVisitor(const Binding& binding, const ProblemInfo& info) : _binding(binding), _info(info), _result(nullptr) {}
	~FormulaBindingVisitor() = default;

 	void Visit(const Formula& lhs) override;
	void Visit(const Tautology& lhs) override;
	void Visit(const Contradiction& lhs) override;
	void Visit(const AtomicFormula& lhs) override;
	void Visit(const Conjunction& lhs) override;
	void Visit(const Disjunction& lhs) override;
	void Visit(const ExistentiallyQuantifiedFormula& lhs) override;
	void Visit(const UniversallyQuantifiedFormula& lhs) override;
	void Visit(const AxiomaticFormula& lhs) override;

	const Formula* _result;
};

////////////////////////////////////////////////////////////////////////////////
//! Processes a term possibly containing bound variables and non-consolidated state variables,
//! consolidating all possible state variables and performing the bindings according to the given variable binding
////////////////////////////////////////////////////////////////////////////////
const Term* bind(const Term& formula, const Binding& binding, const ProblemInfo& info);

class TermBindingVisitor
    : public Loki::BaseVisitor

    , public Loki::Visitor<StateVariable, void, true>
    , public Loki::Visitor<BoundVariable, void, true>
    , public Loki::Visitor<Constant, void, true>
    , public Loki::Visitor<NestedTerm, void, true>
    , public Loki::Visitor<StaticHeadedNestedTerm, void, true>
    , public Loki::Visitor<FluentHeadedNestedTerm, void, true>
    , public Loki::Visitor<UserDefinedStaticTerm, void, true>
    , public Loki::Visitor<AxiomaticTermWrapper, void, true>
    , public Loki::Visitor<AxiomaticTerm, void, true>
    , public Loki::Visitor<AdditionTerm, void, true>
	, public Loki::Visitor<SubtractionTerm, void, true>
	, public Loki::Visitor<MultiplicationTerm, void, true>
	, public Loki::Visitor<DivisionTerm, void, true>
	, public Loki::Visitor<PowerTerm, void, true>
	, public Loki::Visitor<SqrtTerm, void, true>
	, public Loki::Visitor<SineTerm, void, true>
	, public Loki::Visitor<CosineTerm, void, true>
	, public Loki::Visitor<TangentTerm, void, true>
	, public Loki::Visitor<ArcSineTerm, void, true>
	, public Loki::Visitor<ArcCosineTerm, void, true>
	, public Loki::Visitor<ArcTangentTerm, void, true>
	, public Loki::Visitor<ExpTerm, void, true>
	, public Loki::Visitor<MinTerm, void, true>
	, public Loki::Visitor<MaxTerm, void, true>

{
private:
	const Binding& _binding;
	const ProblemInfo& _info;

public:
	TermBindingVisitor(const Binding& binding, const ProblemInfo& info) : _binding(binding), _info(info), _result(nullptr) {}
	~TermBindingVisitor() = default;


	void Visit(const StateVariable& lhs);
	void Visit(const BoundVariable& lhs);
	void Visit(const Constant& lhs);
	void Visit(const NestedTerm& lhs);
	void Visit(const StaticHeadedNestedTerm& lhs);
	void Visit(const FluentHeadedNestedTerm& lhs);
	void Visit(const UserDefinedStaticTerm& lhs);
	void Visit(const AxiomaticTermWrapper& lhs);
	void Visit(const AxiomaticTerm& lhs);
	void Visit(const ArithmeticTerm& lhs);
	void Visit(const AdditionTerm& lhs);
	void Visit(const SubtractionTerm& lhs);
	void Visit(const MultiplicationTerm& lhs);
	void Visit(const DivisionTerm& lhs);
	void Visit(const PowerTerm& lhs);
	void Visit(const SqrtTerm& lhs);
	void Visit(const SineTerm& lhs);
	void Visit(const CosineTerm& lhs);
	void Visit(const TangentTerm& lhs);
	void Visit(const ArcSineTerm& lhs);
	void Visit(const ArcCosineTerm& lhs);
	void Visit(const ArcTangentTerm& lhs);
	void Visit(const ExpTerm& lhs);
	void Visit(const MinTerm& lhs);
	void Visit(const MaxTerm& lhs);
	
	const Term* _result;
};

//! A helper to process lists of subterms
std::vector<const Term*>
bind_subterms(const std::vector<const Term*>& subterms, const Binding& binding, const ProblemInfo& info, std::vector<object_id>& constants);


} } } // namespaces
