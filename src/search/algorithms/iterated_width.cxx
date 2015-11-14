
#include <search/algorithms/iterated_width.hxx>
#include <actions/applicable_action_set.hxx>


namespace fs0 { namespace engines {

FS0IWAlgorithm::FS0IWAlgorithm(const FS0StateModel& model, unsigned initial_max_width, unsigned final_max_width, const NoveltyFeaturesConfiguration& feature_configuration)
	: FS0SearchAlgorithm(model), _current_max_width(initial_max_width), _final_max_width(final_max_width), _feature_configuration(feature_configuration)
{
	setup_base_algorithm(_current_max_width);
}

FS0IWAlgorithm::~FS0IWAlgorithm() {
	if (_algorithm) delete _algorithm;
}


bool FS0IWAlgorithm::search(const State& state, typename FS0SearchAlgorithm::Plan& solution) {
	while(_current_max_width <= _final_max_width) {
		if(_algorithm->search(state, solution)) return true;
		++_current_max_width;
		setup_base_algorithm(_current_max_width);
		solution.clear();
	}
	return false;
}

void FS0IWAlgorithm::setup_base_algorithm(unsigned max_width) {
	if (_algorithm) delete _algorithm;
	std::shared_ptr<SearchNoveltyEvaluator> evaluator = std::make_shared<SearchNoveltyEvaluator>(this->model, _current_max_width, _feature_configuration);
	_algorithm = new BaseAlgorithm(model, OpenList(evaluator));
}

} } // namespaces
