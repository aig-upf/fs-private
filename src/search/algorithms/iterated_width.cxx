
#include <search/algorithms/iterated_width.hxx>
#include <actions/ground_action_iterator.hxx>

namespace fs0 { namespace drivers {
	
template <typename StateModelT>
bool
FS0IWAlgorithm<StateModelT>::search(const State& state, PlanT& solution) {
	while(_current_max_width <= _final_max_width) {
		LPT_INFO("cout", "IW: Starting search with novelty bound of " << _current_max_width);
		if(_algorithm->search(state, solution)) return true;
		++_current_max_width;
		setup_base_algorithm(_current_max_width);
		solution.clear();
	}
	return false;
}

template <typename StateModelT>
void
FS0IWAlgorithm<StateModelT>::setup_base_algorithm(unsigned max_width) {
	std::shared_ptr<SearchNoveltyEvaluator> evaluator = std::make_shared<SearchNoveltyEvaluator>(_model, _current_max_width, _feature_configuration);
	_algorithm = std::unique_ptr<BaseAlgorithm>(new BaseAlgorithm(_model, OpenList(evaluator)));
	lapkt::events::subscribe(*_algorithm, _handlers);
}


} } // namespaces
