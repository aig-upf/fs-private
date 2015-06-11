#pragma once

#include <aptk2/heuristics/novelty/fd_novelty_evaluator.hxx>
#include <state.hxx>

namespace fs0 {

	class GenericStateAdapter  {
	public:

		GenericStateAdapter( const GenericState& s );

		~GenericStateAdapter();
		//! To satisfy the interface requirement in FiniteDomainNoveltyEvaluator::evaluate()
		void	get_valuation( 	std::vector< aptk::VariableIndex >& varnames,
					std::vector< aptk::ValueIndex >& values ) const {
			if ( varnames.size() != _adapted.numAtoms() )
				varnames.resize( _adapted.numAtoms() );
			if ( values.size() != _adapted.numAtoms() )
				values.resize( _adapted.numAtoms() );
			for ( VariableIdx x = 0; x < _adapted.numAtoms(); x++ ) {
				varnames[x] = x;
				values[x] = _adapted.getValue(x);
			}
		}

	protected:

		const GenericState&	_adapted;
	};

	class FS0NoveltyEvaluator : public aptk::FiniteDomainNoveltyEvaluator< GenericStateAdapter > {
	public:
		typedef aptk::FiniteDomainNoveltyEvaluator< GenericStateAdapter > BaseClass;


		unsigned evaluate( const GenericState& s ) {
			GenericStateAdapter adaptee( s );
			return BaseClass::evaluate( adaptee );
		}

		virtual ~FS0NoveltyEvaluator();
	};

}
