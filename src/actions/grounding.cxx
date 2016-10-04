
#include <actions/grounding.hxx>
#include <actions/actions.hxx>
#include <aptk2/tools/logging.hxx>
#include <utils/printers/binding.hxx>
#include <utils/printers/actions.hxx>
#include <utils/config.hxx>
#include <utils/binding_iterator.hxx>
#include <utils/utils.hxx>
#include <languages/fstrips/language.hxx>
#include <unordered_set>

namespace fs0 {


std::vector<const PartiallyGroundedAction*>
ActionGrounder::fully_lifted(const std::vector<const ActionData*>& action_data, const ProblemInfo& info) {
	std::vector<const PartiallyGroundedAction*> lifted;
	// We simply pass an empty binding to each action schema to obtain a fully-lifted PartiallyGroundedAction
	for (const ActionData* data:action_data) {
		lifted.push_back(partial_binding(*data, Binding(data->getSignature().size()), info));
	}
	LPT_INFO("grounding", "Generated " << lifted.size() << " fully-lifted actions");
	return lifted;
}


std::vector<const GroundAction*>
ActionGrounder::fully_ground(const std::vector<const ActionData*>& action_data, const ProblemInfo& info) {
	std::vector<const GroundAction*> grounded;
	unsigned total_num_bindings = 0;
	
	unsigned id = 0;
	for (const ActionData* data:action_data) {
		const Signature& signature = data->getSignature();
		
		// In case the action schema is directly not-lifted, we simply bind it with an empty binding and continue.
		if (signature.empty()) { 
			LPT_INFO("cout", "Grounding action schema '" << data->getName() << "' with no binding");
			LPT_INFO("grounding", "Grounding the following action schema with no binding:\n" << *data << "\n");
			id = ground(id, data, {}, info, grounded);
			++total_num_bindings;
			continue;
		}
		
		utils::binding_iterator binding_generator(signature, info);
		if (binding_generator.ended()) {
			LPT_INFO("cout", "Grounding of schema '" << data->getName() << "' yields no ground action, likely due to a parameter with empty type");
			continue;
		}
		
		unsigned long num_bindings = binding_generator.num_bindings();
		
		if (num_bindings == 0 || num_bindings > MAX_GROUND_ACTIONS) { // num_bindings == 0 would indicate there's been an overflow
			throw TooManyGroundActionsError(num_bindings);
		}
		
		LPT_INFO("cout", "Grounding action schema '" << print::action_data_name(*data) << "' with " << num_bindings << " possible bindings:" << std::flush);
		LPT_INFO("grounding", "Grounding the following action schema with " << num_bindings << " possible bindings:\n" << print::action_data_name(*data) << "\n");
		
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
	
	LPT_INFO("grounding", "Grounding process stats:\n\t* " << grounded.size() << " grounded actions\n\t* " << total_num_bindings - grounded.size() << " pruned actions");
	LPT_INFO("cout", "Grounding process stats:\n\t* " << grounded.size() << " grounded actions\n\t* " << total_num_bindings - grounded.size() << " pruned actions");
	return grounded;
}

unsigned
ActionGrounder::ground(unsigned id, const ActionData* data, const Binding& binding, const ProblemInfo& info, std::vector<const GroundAction*>& grounded) {
	LPT_DEBUG("grounding", "Binding: " << print::binding(binding, data->getSignature()));
	
	if (GroundAction* ground = full_binding(id, *data, binding, info)) {
// 		LPT_DEBUG("grounding", "Binding " << print::binding(binding, data->getSignature()) << " generated grounded action:\n" << *ground);
		grounded.push_back(ground);
		return id + 1;
	} else {
		LPT_DEBUG("grounding", "Binding " << print::binding(binding, data->getSignature()) << " generates a statically non-applicable grounded action");
	}
	return id;
}

ActionData*
ActionGrounder::process_action_data(const ActionData& action_data, const ProblemInfo& info) {
	Binding binding; // An empty binding
	auto precondition = action_data.getPrecondition()->bind(binding, info);
	if (precondition->is_contradiction()) {
		throw std::runtime_error("The precondition of the action schema is (statically) unsatisfiable!");
	}
	
	std::vector<const fs::ActionEffect*> effects;
	for (const fs::ActionEffect* effect:action_data.getEffects()) {
		effects.push_back(effect->bind(binding, info));
	}
	return new ActionData(action_data.getId(), action_data.getName(), action_data.getSignature(), action_data.getParameterNames(), precondition, effects);
}



GroundAction*
ActionGrounder::full_binding(unsigned id, const ActionData& action_data, const Binding& binding, const ProblemInfo& info) {
	assert(binding.is_complete()); // Grounding only possible for full bindings
	const fs::Formula* precondition = action_data.getPrecondition()->bind(binding, info);
	if (precondition->is_contradiction()) {
		delete precondition;
		return nullptr;
	}
	
	std::vector<const fs::ActionEffect*> effects;
	for (const fs::ActionEffect* effect:action_data.getEffects()) {
		effects.push_back(effect->bind(binding, info));
	}
	
	return new GroundAction(id, action_data, binding, precondition, effects);
}


std::vector<const PartiallyGroundedAction*>
ActionGrounder::compile_action_parameters_away(const PartiallyGroundedAction* schema, unsigned effect_idx, const ProblemInfo& info) {
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

		if (PartiallyGroundedAction* grounded_action = partial_binding(schema->getActionData(), binding, info)) {
			grounded.push_back(grounded_action);
		}
	}
	
	return grounded;
}


const std::vector<const fs::ActionEffect*>
ActionGrounder::compile_nested_fluents_away(const fs::ActionEffect* effect, const ProblemInfo& info) {

	const fs::FluentHeadedNestedTerm* head = dynamic_cast<const fs::FluentHeadedNestedTerm*>(effect->lhs());
	if (!head) return { new fs::ActionEffect(*effect) }; // There cannot be nested fluents, so we can safely return the same effect
	
	LPT_DEBUG("main", "Compiling away nested fluents in the head of effect \"" << *effect);

	
	// Let us find the index of the first non-constant subterm
	auto original_subterms = head->getSubterms();
	unsigned i = 0;
	for (; i < original_subterms.size(); ++i) {
		if (!dynamic_cast<const fs::Constant*>(original_subterms[i])) break;
	}
	assert(i < original_subterms.size()); // We necessarily have a non-const subterm, otherwise the head wouldn't be a fluent nested term
	
	
	std::vector<const fs::ActionEffect*> compiled;
	
	const fs::Term* subterm_to_replace = original_subterms[i];
	for (ObjectIdx value:info.getTypeObjects(subterm_to_replace->getType())) {
		auto subterms = Utils::clone(original_subterms);
		delete subterms[i];
		
		subterms[i] = new fs::IntConstant(value);
		auto extra_condition = new fs::EQAtomicFormula({subterm_to_replace->clone(), subterms[i]->clone()});
		auto new_condition = effect->condition()->conjunction(extra_condition);
		delete extra_condition;
		
		auto tmp_head = new fs::FluentHeadedNestedTerm(head->getSymbolId(), subterms);
		auto processed_head = tmp_head->bind({}, info);
		delete tmp_head;
		
		// Now the recursive call
		auto eff = new fs::ActionEffect(processed_head, effect->rhs()->clone(), new_condition);
		auto recursively_compiled = compile_nested_fluents_away(eff, info);
		delete eff;
		compiled.insert(compiled.end(), recursively_compiled.begin(), recursively_compiled.end());
	}
	
	return compiled;
}
	
	
// 	WORK_IN_PROGRESS("Still to be though out: how to dynamically add parameters so that afterwards the comparisons between lifted action IDs are correct");
	
	/*
	
	std::vector<const fs::Term*> subterms;
	for (const fs::Term* subterm:head->getSubterms()) {
		if (dynamic_cast<const fs::Constant*>(subterm)) subterms.push_back(subterm->clone()); // A constant subterm is OK, no need to replace it by anything
		
		// Otherwise, we must have a subterm which is a state variable or a nested fluent, which is 
		// preventing the whole effect head from being an actual state variable itself. We'll compile it away
		// as a condition of the effect, i.e. if, for instance the effect is of the form f(g(c)) := t,
		// we take the first non-const subterm in the head, "g(c)", and rewrite the effect to get rid of it by using a conditional effect,
		// e.g. g(c)=d --> f(d) := t.
		
		// For each possible value of the non-const subterm, we'll have an additional conditional effect.
		for (ObjectIdx value:info.getTypeObjects(subterm->getType())) {
			auto constant = new fs::IntConstant(value);
// 			std::vector<const fs::Term*> subterms{subterm->clone(), new fs::IntConstant(value)};
			auto extra_condition = new fs::EQAtomicFormula({subterm->clone(), constant});
			
		}
		
	}
	
	
	
	
	
	std::vector<const fs::ActionEffect> compiled;

	std::vector<const fs::Term*> subterms = collect_effect_non_constant_subterms(effect);
	if (subterms.empty()) { // The received effect is already fluent-less
		compiled.push_back(effect);
		return compiled;
	}
	
	
	// Otherwise, suppose the effect is of the form f(g(c)) := t.
	// We take the first non-const subterm in the head, e.g. g(c), and rewrite the effect to get rid of it by using a conditional effect,
	// e.g. g(c)=d --> f(d) := t.
	const fs::Term* subterm = subterms.at(0); // Simply take the first state variable in the head.
	
	// For each possible value of the non-const subterm, we'll have an additional conditional effect.
	for (ObjectIdx value:info.getTypeObjects(subterm->getType())) {
		
		std::vector<const fs::Term*> subterms{subterm->clone(), new fs::IntConstant(value)};
		auto extra_condition = new fs::EQAtomicFormula(subterms);
		
		effect->add_condition(extra_condition);
		effect->lhs()->replace(subterm, extra_condition);
		effect->rhs()->replace(subterm, extra_condition);
	}
	
	// If there are still other non-const subterms, we proceed recursively
	
	std::vector<const PartiallyGroundedAction*> rewritten;
	
	
	WORK_IN_PROGRESS("Still to be though out: how to dynamically add parameters so that afterwards the comparisons between lifted action IDs are correct");
	
	return rewritten;
	
}

std::vector<const fs::Term*>
collect_effect_non_constant_subterms(const fs::ActionEffect* effect) {
	std::vector<const fs::Term*> subterms;
	const fs::FluentHeadedNestedTerm* head = dynamic_cast<const fs::FluentHeadedNestedTerm*>(effect->lhs());
	if (head) {
		for (const fs::Term* subterm:head->getSubterms()) {
			if (!dynamic_cast<const fs::Constant*>(subterm)) {
				subterms.push_back(subterm);
			}
		}
	}
	return subterms;
}

*/
PartiallyGroundedAction*
ActionGrounder::partial_binding(const ActionData& action_data, const Binding& binding, const ProblemInfo& info) {
	const fs::Formula* precondition = action_data.getPrecondition()->bind(binding, info);
	if (precondition->is_contradiction()) {
		delete precondition;
		return nullptr;
	}
	
	std::vector<const fs::ActionEffect*> effects;
	for (const fs::ActionEffect* effect:action_data.getEffects()) {
		effects.push_back(effect->bind(binding, info));
	}
	
	return new PartiallyGroundedAction(action_data, binding, precondition, effects);
}

GroundAction*
ActionGrounder::bind(const PartiallyGroundedAction& action, const Binding& binding, const ProblemInfo& info) {
	Binding full(action.getBinding());
	full.merge_with(binding); // TODO We should bind not from the action data but from the partially bound action itself.
	return full_binding(GroundAction::invalid_action_id, action.getActionData(), full, info);
}


} // namespaces
