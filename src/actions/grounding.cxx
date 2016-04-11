
#include <problem.hxx>
#include <actions/grounding.hxx>
#include <actions/actions.hxx>
#include <problem_info.hxx>
#include <utils/logging.hxx>
#include <utils/cartesian_iterator.hxx>
#include <utils/printers/binding.hxx>
#include <utils/printers/actions.hxx>
#include <utils/config.hxx>
#include <utils/binding_iterator.hxx>
#include <utils/utils.hxx>
#include <languages/fstrips/language.hxx>
#include <unordered_set>

namespace fs0 {



void ActionGrounder::selective_grounding(const std::vector<const ActionData*>& action_data, const ProblemInfo& info, Problem& problem) {
	// ATM we simply generate all grounded and all lifted actions, but TODO this decision should be made at the engine level.
	if (!Config::instance().doLiftedPlanning()) problem.setGroundActions(fully_ground(action_data, info));
	problem.setPartiallyGroundedActions(fully_lifted(action_data, info));
}


std::vector<const PartiallyGroundedAction*> ActionGrounder::fully_lifted(const std::vector<const ActionData*>& action_data, const ProblemInfo& info) {
	std::vector<const PartiallyGroundedAction*> lifted;
	// We simply pass an empty binding to each action schema to obtain a fully-lifted PartiallyGroundedAction
	for (const ActionData* data:action_data) {
		lifted.push_back(partial_binding(*data, Binding(data->getSignature().size()), info));
	}
	FINFO("grounding", "Generated " << lifted.size() << " fully-lifted actions");
	return lifted;
}


std::vector<const GroundAction*> ActionGrounder::fully_ground(const std::vector<const ActionData*>& action_data, const ProblemInfo& info) {
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
		
		utils::binding_iterator binding_generator(signature, info);
		int num_bindings = binding_generator.num_bindings();
		
		std::cout <<  "Grounding action schema '" << print::action_data_name(*data) << "' with " << num_bindings << " possible bindings:\n\t" << std::flush;
		FINFO("grounding", "Grounding the following action schema with " << num_bindings << " possible bindings:\n" << print::action_data_name(*data) << "\n");
		
		float onepercent = ((float)num_bindings / 100);
		int progress = 0;
		unsigned i = 0;
		for (; !binding_generator.ended(); ++binding_generator) {
			id = ground(id, data, *binding_generator, info, grounded);
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
	assert(binding.is_complete()); // Grounding only possible for full bindings
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


std::vector<const PartiallyGroundedAction*> ActionGrounder::flatten_effect_head(const PartiallyGroundedAction* schema, unsigned effect_idx, const ProblemInfo& info) {
	const fs::ActionEffect* effect = schema->getEffects().at(effect_idx);
	
	auto head_parameters = Utils::filter_by_type<const fs::BoundVariable*>(effect->lhs()->all_terms());
	
	// If there are no parameters on the effect head, we simply return a vector with a clone of the action
	if (head_parameters.empty()) {
		return { new PartiallyGroundedAction(*schema) };
	}
	
	// Otherwise, we ground the whole effect wrt the head parameters, and return all the resulting effects
	std::vector<const PartiallyGroundedAction*> grounded;
	std::vector<TypeIdx> types(schema->getSignature().size(), INVALID_TYPE); // Initialize a type vector with all types being invalid
	for (const fs::BoundVariable* parameter:head_parameters) {
		types.at(parameter->getVariableId()) = parameter->getType();
	}
	
	utils::binding_iterator binding_generator(types, info);
	for (; !binding_generator.ended(); ++binding_generator) {
		// We generate the binding that results from merging the binding of the partially grounded action with the one
		// that we create to turn the head of the given effect into a state variable
		Binding binding = schema->getBinding(); // Copy the object
		binding.merge_with(*binding_generator);
		grounded.push_back(partial_binding(schema->getActionData(), binding, info));
	}
	
// 	return compile_nested_fluents_away(grounded, info);
	return grounded;
}

/*
std::vector<const PartiallyGroundedAction*> ActionGrounder::compile_nested_fluents_away(const std::vector<const PartiallyGroundedAction*>& actions, const ProblemInfo& info) {
	
	// We re-write the actions to compile away nested fluents in the effects' heads into additional action parameters.
	// Thus, an effect "tile(blank) := t" with both 'tile' and 'blank' being fluent function symbols will get rewritten
	// into k actions with an extra parameter param_blank, extra precondition param_blank = blank, and effect tile(param_blank) := t
	std::vector<const PartiallyGroundedAction*> rewritten;

	for (const PartiallyGroundedAction* action:actions) {
		
		std::vector<const fs::StateVariable*> subvars = collect_effect_head_variables(action);
		if (subvars.empty()) {
			rewritten.push_back(action);
			continue; // No need for further processing of the action
		}
		
		
		PartiallyGroundedAction* processed = new PartiallyGroundedAction(*action);
		
		unsigned num_params = processed->numParameters();
		
		
		std::unordered_set<const fs::StateVariable*> unique_subvars(subvars.begin(), subvars.end());
		for (const fs::StateVariable* statevar:unique_subvars) {
			// We have a state variable f(c) in some subterm of the effect LHS
			auto extra_param = new fs::BoundVariable(num_params++, statevar->getType());
			processed->addParameter(extra_param);
			processed->replaceTerm(statevar, extra_param);
			
			std::vector<const fs::Term*> subterms{statevar->clone(), extra_param};
			auto extra_precondition = new fs::EQAtomicFormula(subterms);
			processed->addPrecondition(extra_precondition);
		}
		
		rewritten.push_back(processed);
		delete action; // ???
	}
	
	WORK_IN_PROGRESS("Still to be though out: how to dynamically add parameters so that afterwards the comparisons between lifted action IDs are correct");
	
	return rewritten;
}
*/

std::vector<const fs::StateVariable*> ActionGrounder::collect_effect_head_variables(const PartiallyGroundedAction* action) {
	std::vector<const fs::StateVariable*> statevars;
		
	for (const fs::ActionEffect* effect:action->getEffects()) {
		
		for (const fs::StateVariable* statevar:Utils::filter_by_type<const fs::StateVariable*>(effect->lhs()->all_terms())) {
			if (statevar == effect->lhs()) continue; // We just want to collect subterms' state variables
			statevars.push_back(statevar);
		}
	}
	return statevars;
}

PartiallyGroundedAction* ActionGrounder::partial_binding(const ActionData& action_data, const Binding& binding, const ProblemInfo& info) {
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
	Binding full(action.getBinding());
	full.merge_with(binding); // TODO We should bind not from the action data but from the partially bound action itself.
	return full_binding(GroundAction::invalid_action_id, action.getActionData(), full, info);
}


} // namespaces
