
#pragma once

#include <utils/visitor.hxx>
#include <languages/fstrips/formulae.hxx>

//!
//!
//!

namespace fs0 { class Binding; class ProblemInfo; }

namespace fs0 { namespace language { namespace fstrips {
	

//! Processes a formula possibly containing bound variables and non-consolidated state variables,
//! consolidating all possible state variables and performing the bindings according to the given variable binding
const Formula* bind(const Formula& formula, const Binding& binding, const ProblemInfo& info);
	
class BindingVisitor
    : public Loki::BaseVisitor
    , public Loki::Visitor<Formula, void, true>
    , public Loki::Visitor<Tautology, void, true>
	, public Loki::Visitor<Contradiction, void, true>
	, public Loki::Visitor<AtomicFormula, void, true>
	, public Loki::Visitor<Conjunction, void, true>
	, public Loki::Visitor<ExistentiallyQuantifiedFormula, void, true>
	, public Loki::Visitor<AxiomaticFormula, void, true>
	
{
public:
	BindingVisitor(const Binding& binding, const ProblemInfo& info) : _binding(binding), _info(info) {}
	~BindingVisitor() = default;
	
 	void Visit(const Formula& lhs) override { throw UnimplementedFeatureException(""); }
	void Visit(const Tautology& lhs) override { _result = new Tautology; }
	void Visit(const Contradiction& lhs) override { _result = new Contradiction; }
	void Visit(const AtomicFormula& lhs) override;
	void Visit(const Conjunction& lhs) override;
	void Visit(const ExistentiallyQuantifiedFormula& lhs) override;
	void Visit(const AxiomaticFormula& lhs) override;

	const Binding& _binding;
	const ProblemInfo& _info;
	const Formula* _result;
};


} } } // namespaces
