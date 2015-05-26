#include <heuristics/fs0_novelty_evaluator.hxx>

namespace fs0 {

	GenericStateAdapter::GenericStateAdapter( const GenericState& s ) 
		: _adapted( s ) {}

	GenericStateAdapter::~GenericStateAdapter() { }

	FS0NoveltyEvaluator::~FS0NoveltyEvaluator() { }

}
