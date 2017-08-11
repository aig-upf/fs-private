
#include <algorithm>

#include <languages/fstrips/operations/interpretation.hxx>
#include <languages/fstrips/terms.hxx>
#include <languages/fstrips/builtin.hxx>
#include <utils/binding.hxx>
#include <lapkt/tools/logging.hxx>
#include <problem_info.hxx>

namespace fs0 { namespace language { namespace fstrips {


//! Quick helpers
VariableIdx interpret_variable(const Term& element, const State& assignment) { return interpret_variable(element, assignment, Binding::EMPTY_BINDING); }
VariableIdx interpret_variable(const Term& element, const PartialAssignment& assignment) { return interpret_variable(element, assignment, Binding::EMPTY_BINDING); }


template <typename AssignmentT>
VariableIdx interpret_variable(const Term& element, const AssignmentT& assignment, const Binding& binding) {
	VariableInterpretationVisitor<AssignmentT> visitor(assignment, binding);
	element.Accept(visitor);
	return visitor._result;
}

// Explicit instantiations
template VariableIdx interpret_variable(const Term& element, const State& assignment, const Binding& binding);
template VariableIdx interpret_variable(const Term& element, const PartialAssignment& assignment, const Binding& binding);


template <typename AssignmentT>
void VariableInterpretationVisitor<AssignmentT>::
Visit(const StateVariable& lhs) {
	_result = lhs.getValue();
}

template <typename AssignmentT>
void VariableInterpretationVisitor<AssignmentT>::
Visit(const FluentHeadedNestedTerm& lhs) {
	const auto& subterms = lhs.getSubterms();
	std::vector<object_id> interpreted(subterms.size());
	NestedTerm::interpret_subterms(subterms, _assignment, _binding, interpreted);
    try {
        _result = ProblemInfo::getInstance().resolveStateVariable(lhs.getSymbolId(), interpreted);
    }
    catch ( std::exception& e ) {
        LPT_INFO( "main", "Runtime Error: Could not resolve state variable for nested term!");
        //LPT_INFO( "main", "Assignment: " << _assignment );
        //LPT_INFO( "main", "Binding:" << _binding);
        LPT_INFO( "main", "Term: " << lhs);
        std::cerr << "Runtime Error: check main.log for details" << std::endl;
        assert(false);// MRJ: to stop the debugger here
        std::exit(1);
    }

}


template <typename AssignmentT>
void VariableInterpretationVisitor<AssignmentT>::
Visit(const UserDefinedStaticTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }

template <typename AssignmentT>
void VariableInterpretationVisitor<AssignmentT>::
Visit(const AxiomaticTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }


template <typename AssignmentT>
void VariableInterpretationVisitor<AssignmentT>::
Visit(const AdditionTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }


template <typename AssignmentT>
void VariableInterpretationVisitor<AssignmentT>::
Visit(const SubtractionTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }


template <typename AssignmentT>
void VariableInterpretationVisitor<AssignmentT>::
Visit(const MultiplicationTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }

template <typename AssignmentT>
void VariableInterpretationVisitor<AssignmentT>::
Visit(const DivisionTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }

template <typename AssignmentT>
void VariableInterpretationVisitor<AssignmentT>::
Visit(const PowerTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }

template <typename AssignmentT>
void VariableInterpretationVisitor<AssignmentT>::
Visit(const SqrtTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }

template <typename AssignmentT>
void VariableInterpretationVisitor<AssignmentT>::
Visit(const SineTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }

template <typename AssignmentT>
void VariableInterpretationVisitor<AssignmentT>::
Visit(const CosineTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }

template <typename AssignmentT>
void VariableInterpretationVisitor<AssignmentT>::
Visit(const TangentTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }

template <typename AssignmentT>
void VariableInterpretationVisitor<AssignmentT>::
Visit(const ArcSineTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }

template <typename AssignmentT>
void VariableInterpretationVisitor<AssignmentT>::
Visit(const ArcCosineTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }

template <typename AssignmentT>
void VariableInterpretationVisitor<AssignmentT>::
Visit(const ArcTangentTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }

template <typename AssignmentT>
void VariableInterpretationVisitor<AssignmentT>::
Visit(const ExpTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }

template <typename AssignmentT>
void VariableInterpretationVisitor<AssignmentT>::
Visit(const MinTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }

template <typename AssignmentT>
void VariableInterpretationVisitor<AssignmentT>::
Visit(const MaxTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }



} } } // namespaces
