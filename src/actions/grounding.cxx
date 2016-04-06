
#include <problem.hxx>
#include <actions/grounding.hxx>
#include <actions/actions.hxx>
#include <problem_info.hxx>
#include <utils/logging.hxx>
#include <utils/cartesian_iterator.hxx>
#include <utils/printers/binding.hxx>
#include <utils/printers/actions.hxx>
#include <utils/config.hxx>
#include <languages/fstrips/language.hxx>

namespace fs0 {



void ActionGrounder::selective_grounding(const std::vector<const ActionData*>& action_data, const ProblemInfo& info, Problem& problem) {
	if (Config::instance().doLiftedPlanning()) {
		problem.setPartiallyGroundedActions(no_grounding(action_data, info));
	} else {
		problem.setGroundActions(full_grounding(action_data, info));
	}
}


std::vector<const PartiallyGroundedAction*> ActionGrounder::no_grounding(const std::vector<const ActionData*>& action_data, const ProblemInfo& info) {
	std::vector<const PartiallyGroundedAction*> lifted;
	// We simply pass an empty binding to each action schema to obtain a fully-lifted PartiallyGroundedAction
	for (const ActionData* data:action_data) {
		lifted.push_back(partial_binding(*data, Binding(data->getSignature().size()), info));
	}
	FINFO("grounding", "Generated " << lifted.size() << " fully-lifted actions");
	return lifted;
}


std::vector<const GroundAction*> ActionGrounder::full_grounding(const std::vector<const ActionData*>& action_data, const ProblemInfo& info) {
	std::vector<const GroundAction*> grounded;
	unsigned total_num_bindings = 0;
	
	unsigned id = 0;
	for (const ActionData* data:action_data) {
		const Signature& signature = data->getSignature();
		
		// In case the action schema is directly not-lifted, we simply bind it with an empty binding and continue.
		if (signature.empty()) { 
			std::cout <<  "Grounding action schema '" << data->getName() << "' with no binding" << std::endl;
			FINFO("grounding", "Grounding the following action schema with no binding:\n" << *data << "\n");
			id = ground(id, data, {}, info, grounded);
			++total_num_bindings;
			continue;
		}
		
		
		std::vector<const ObjectIdxVector*> values = info.getSignatureValues(signature);
		int num_bindings = std::accumulate(values.begin(), values.end(), 1, [](int a, const ObjectIdxVector* b) { return a * b->size(); });
		
		std::cout <<  "Grounding action schema '" << print::action_data_name(*data) << "' with " << num_bindings << " possible bindings:\n\t" << std::flush;
		FINFO("grounding", "Grounding the following action schema with " << num_bindings << " possible bindings:\n" << print::action_data_name(*data) << "\n");
		
		float onepercent = ((float)num_bindings / 100);
		int progress = 0;
		
		utils::cartesian_iterator cross_product(std::move(values));
		unsigned i = 0;
		for (; !cross_product.ended(); ++cross_product) {
			id = ground(id, data, Binding(*cross_product), info, grounded);
			++i;
			
			// Print 5%, 10%, 15%, ... progress indicators
			while (i / onepercent > progress) {
				++progress;
				if (progress % 5 == 0) std::cout << progress << "%, " << std::flush;
			}
			++total_num_bindings;
		}
		std::cout << std::endl;
	}
	
	FINFO("grounding", "Grounding process stats:\n\t* " << grounded.size() << " grounded actions\n\t* " << total_num_bindings - grounded.size() << " pruned actions");
	std::cout << "Grounding process stats:\n\t* " << grounded.size() << " grounded actions\n\t* " << total_num_bindings - grounded.size() << " pruned actions" << std::endl;

	return grounded;
}

unsigned ActionGrounder::ground(unsigned id, const ActionData* data, const Binding& binding, const ProblemInfo& info, std::vector<const GroundAction*>& grounded) {
	FDEBUG("grounding", "Binding: " << print::binding(binding, data->getSignature()));
	
	if (GroundAction* ground = full_binding(id, *data, binding, info)) {
// 		FDEBUG("grounding", "Binding " << print::binding(binding, data->getSignature()) << " generated grounded action:\n" << *ground);
		grounded.push_back(ground);
		return id + 1;
	} else {
		FDEBUG("grounding", "Binding " << print::binding(binding, data->getSignature()) << " generates a statically non-applicable grounded action");
	}
	return id;
}

ActionData* ActionGrounder::process_action_data(const ActionData& action_data, const ProblemInfo& info) {
	Binding binding; // An empty binding
	auto precondition = action_data.getPrecondition()->bind(binding, info);
	if (precondition->is_contradiction()) {
		throw std::runtime_error("The precondition of the action schema is (statically) unsatisfiable!");
	}
	
	std::vector<fs::ActionEffect::cptr> effects;
	for (const fs::ActionEffect::cptr effect:action_data.getEffects()) {
		effects.push_back(effect->bind(binding, info));
	}
	return new ActionData(action_data.getId(), action_data.getName(), action_data.getSignature(), action_data.getParameterNames(), precondition, effects);
}



GroundAction* ActionGrounder::full_binding(unsigned id, const ActionData& action_data, const Binding& binding, const ProblemInfo& info) {
	assert(binding.is_complete()); // Grounding only possible for full bingdings
	const fs::Formula* precondition = action_data.getPrecondition()->bind(binding, info);
	if (precondition->is_contradiction()) {
		delete precondition;
		return nullptr;
	}
	
	std::vector<fs::ActionEffect::cptr> effects;
	for (const fs::ActionEffect::cptr effect:action_data.getEffects()) {
		effects.push_back(effect->bind(binding, info));
	}
	
	return new GroundAction(id, action_data, binding, precondition, effects);
}

PartiallyGroundedAction* ActionGrounder::partial_binding(const ActionData& action_data, const Binding& binding, const ProblemInfo& info) {
	assert(!binding.is_complete()); // Grounding only possible for full bingdings
	const fs::Formula* precondition = action_data.getPrecondition()->bind(binding, info);
	if (precondition->is_contradiction()) {
		delete precondition;
		return nullptr;
	}
	
	std::vector<fs::ActionEffect::cptr> effects;
	for (const fs::ActionEffect::cptr effect:action_data.getEffects()) {
		effects.push_back(effect->bind(binding, info));
	}
	
	return new PartiallyGroundedAction(action_data, binding, precondition, effects);
}

GroundAction* ActionGrounder::bind(const PartiallyGroundedAction& action, const Binding& binding, const ProblemInfo& info) {
	return full_binding(GroundAction::invalid_action_id, action.getActionData(), binding, info);
}


} // namespaces
