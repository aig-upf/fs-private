
#include <unordered_set>

#include <lapkt/tools/logging.hxx>

#include <fs/core/problem_info.hxx>
#include <fs/core/actions/grounding.hxx>
#include <fs/core/actions/actions.hxx>
#include <fs/core/utils/printers/binding.hxx>
#include <fs/core/utils/printers/actions.hxx>
#include <fs/core/utils/config.hxx>
#include <fs/core/utils/binding_iterator.hxx>
#include <fs/core/utils/utils.hxx>
#include <fs/core/utils/loader.hxx>
#include <fs/core/languages/fstrips/language.hxx>
#include <fs/core/languages/fstrips/operations.hxx>


namespace fs0 {


std::vector<const fs::ActionEffect*>
_bind_effects(const ActionData& action_data, const Binding& binding, const ProblemInfo& info) {
	std::vector<const fs::ActionEffect*> effects;
	for (const fs::ActionEffect* effect:action_data.getEffects()) {
		if (const fs::ActionEffect* bound = effect->bind(binding, info)) {
			effects.push_back(bound);
		}
	}

	if (effects.empty()) {
		LPT_INFO("grounding", "WARNING - " <<  action_data << " with binding " << binding << " has no applicable effects");
		LPT_DEBUG("cout", "WARNING - " <<  action_data << " with binding " << binding << " has no applicable effects");
	}
	return effects;
}

PartiallyGroundedAction*
_partial_binding(const ActionData& action_data, const Binding& binding, const ProblemInfo& info) {
	const fs::Formula* precondition = fs::bind(*action_data.getPrecondition(), binding, info);
	if (precondition->is_contradiction()) {
		delete precondition;
		return nullptr;
	}

	auto effects = _bind_effects(action_data, binding, info);
	if (effects.empty()) {
		delete precondition;
		return nullptr;
	}

	return new PartiallyGroundedAction(action_data, binding, precondition, effects);
}


//! Process the action schema with a given parameter binding and return the corresponding GroundAction
//! A nullptr is returned if the action is detected to be statically non-applicable
GroundAction*
_full_binding(unsigned id, const ActionData& action_data, const Binding& binding, const ProblemInfo& info, bool bind_effects) {
	assert(binding.is_complete()); // Grounding only possible for full bindings
	const fs::Formula* precondition = fs::bind(*action_data.getPrecondition(), binding, info);
	if (precondition->is_contradiction()) {
		delete precondition;
		return nullptr;
	}

	std::vector<const fs::ActionEffect*> effects;
	if (bind_effects) {
		effects = _bind_effects(action_data, binding, info);
		if (effects.empty() && !action_data.hasProceduralEffects()) {
			delete precondition;
			return nullptr;
		}
	}

	if (action_data.hasProceduralEffects())
		return new ProceduralAction(id, action_data, binding, precondition, effects);

	return new GroundAction(id, action_data, binding, precondition, effects);
}

//! Grounds the set of given action schemata with all parameter groundings that induce no false preconditions
//! Returns the new set of grounded actions
unsigned
_ground(unsigned id, const ActionData* data, const Binding& binding, const ProblemInfo& info, std::vector<const GroundAction*>& grounded, bool bind_effects) {
// 	LPT_DEBUG("grounding", "Binding: " << print::binding(binding, data->getSignature()));

	if (GroundAction* ground = _full_binding(id, *data, binding, info, bind_effects)) {
		LPT_EDEBUG("groundings", "\t" << *ground);
		grounded.push_back(ground);
		return id + 1;
	} else {
		LPT_DEBUG("grounding", "Binding " << print::binding(binding, data->getSignature()) << " generates a statically non-applicable grounded action");
	}
	return id;
}



std::vector<const PartiallyGroundedAction*>
ActionGrounder::fully_lifted(const std::vector<const ActionData*>& action_data, const ProblemInfo& info) {
	std::vector<const PartiallyGroundedAction*> lifted;
	// We simply pass an empty binding to each action schema to obtain a fully-lifted PartiallyGroundedAction
	for (const ActionData* data:action_data) {
		lifted.push_back(_partial_binding(*data, Binding(data->getSignature().size()), info));
	}
	LPT_INFO("grounding", "Generated " << lifted.size() << " fully-lifted actions");
	return lifted;
}

// TODO - Might want to move this somewhere else and make it public
std::vector<object_id>
deserialize_typed_objects(const ProblemInfo& info, const std::string& line, const Signature& signature) {
	std::vector<type_id> sym_signature_types = info.get_type_ids(signature);
	return Serializer::deserialize_line( line, sym_signature_types, ",");
}

//! Loads a set of ground action from the given data directory, if they exist, or else returns an empty vector
std::vector<const GroundAction*>
_loadGroundActionsIfAvailable(const ProblemInfo& info, const std::vector<const ActionData*>& action_data) {
	std::vector<const GroundAction*> grounded;
	if (action_data.empty()) return grounded;

	std::string filename = info.getDataDir() + "/groundings.data";
	std::ifstream is(filename);

    if (!is.good()) { // File groundings.data does not exist
		return grounded;
	}

	LPT_INFO("grounding", "Loading the list of reachable ground actions from \"" << filename << "\"");
	LPT_DEBUG("cout", "Loading the list of reachable ground actions from \"" << filename << "\"");

	unsigned current_schema_groundings = 0;
	unsigned id = 0;
	unsigned schema_id = -1;
	const ActionData* current = action_data[0];
	std::string line;

	while (std::getline(is, line)) {
		if (line.length() > 0 && line[0] == '#') { // We switch to the next action schema

			++schema_id;
			if (schema_id >= action_data.size()) {
				throw std::runtime_error("The number of action schemas in the groundings file does not match that in the problem description");
			}

			if (schema_id > 0) {
				LPT_INFO("grounding", "Action schema \"" << current->getName() << "\" results in " << current_schema_groundings << " grounded actions");
				LPT_DEBUG("cout", "Action schema \"" << current->getName() << "\" results in " << current_schema_groundings << " grounded actions");
			}

			current = action_data[schema_id];
			current_schema_groundings = 0;
			continue;
		}

		std::vector<object_id> deserialized = deserialize_typed_objects(info, line, current->getSignature());
		if (deserialized.empty()) {
			LPT_INFO("grounding", "Grounding action schema '" << current->getName() << "' with no binding");
			LPT_DEBUG("cout", "Grounding action schema '" << current->getName() << "' with no binding");
			id = _ground(id, current, Binding::EMPTY_BINDING, info, grounded, true);
		} else {
			Binding binding(std::move(deserialized));
			id = _ground(id, current, binding, info, grounded, true);
		}
		++current_schema_groundings;
	}
	LPT_INFO("grounding", "Action schema \"" << current->getName() << "\" results in " << current_schema_groundings << " grounded actions");
	LPT_INFO("grounding", "Grounding process stats:\t" << grounded.size() << " grounded actions");
	LPT_DEBUG("cout", "Action schema \"" << current->getName() << "\" results in " << current_schema_groundings << " grounded actions");
	LPT_DEBUG("cout", "Grounding process stats:\t" << grounded.size() << " grounded actions");
	return grounded;
}

std::vector<const GroundAction*>
_ground_all_elements(const std::vector<const ActionData*>& action_data, const ProblemInfo& info, bool bind_effects) {
	std::vector<const GroundAction*> grounded;

	unsigned total_num_bindings = 0;

	unsigned id = 0;
	for (const ActionData* data:action_data) {
		unsigned grounded_0 = grounded.size();
		const Signature& signature = data->getSignature();

		// In case the action schema is directly not-lifted, we simply bind it with an empty binding and continue.
		if (signature.empty()) {
			LPT_DEBUG("cout", "Grounding schema '" << data->getName() << "' with no binding");
			LPT_INFO("grounding", "Grounding the following schema with no binding:" << *data << "\n");
			id = _ground(id, data, Binding::EMPTY_BINDING, info, grounded, bind_effects);
			++total_num_bindings;
			continue;
		}

		utils::binding_iterator binding_generator(signature, info);
		if (binding_generator.ended()) {
			LPT_DEBUG("cout", "Grounding of schema '" << data->getName() << "' yields no ground element, likely due to a parameter with empty type");
			LPT_INFO("grounding", "Grounding of schema '" << data->getName() << "' yields no ground element, likely due to a parameter with empty type");
			continue;
		}

		unsigned long num_bindings = binding_generator.num_bindings();

		LPT_DEBUG("cout", "Grounding schema '" << print::action_data_name(*data) << "' with " << num_bindings << " possible bindings" << std::flush);
		LPT_INFO("grounding", "Grounding the following schema with " << num_bindings << " possible bindings:" << print::action_data_name(*data));

		if (num_bindings == 0 || num_bindings > ActionGrounder::MAX_GROUND_ACTIONS) { // num_bindings == 0 would indicate there's been an overflow
			//throw TooManyGroundActionsError(num_bindings);
			LPT_INFO("grounding", "WARNING - The number of ground elements is too high: " << num_bindings);
			LPT_DEBUG("cout", "WARNING - The number of ground elements is too high: " << num_bindings);
		}

// 		float onepercent = ((float)num_bindings / 100);
// 		int progress = 0;
// 		unsigned i = 0;
		for (; !binding_generator.ended(); ++binding_generator) {
			id = _ground(id, data, *binding_generator, info, grounded, bind_effects);
// 			++i;

			// Print 5%, 10%, 15%, ... progress indicators
			/*
			 * NOTE This is too expensive for problems with many ground actions!
			while (i / onepercent > progress) {
				++progress;
				if (progress % 5 == 0) std::cout << progress << "%, " << std::flush;
			}
			*/
			++total_num_bindings;
		}
// 		std::cout << std::endl;
		LPT_INFO("grounding", "Schema \"" << print::action_data_name(*data) << "\" results in " << grounded.size() - grounded_0 << " grounded elements");
		LPT_DEBUG("cout", "Schema \"" << print::action_data_name(*data) << "\" results in " << grounded.size() - grounded_0 << " grounded elements");
	}

	LPT_INFO("grounding", "Grounding stats:\n\t* " << grounded.size() << " grounded elements\n\t* " << total_num_bindings - grounded.size() << " pruned elements");
	LPT_DEBUG("cout", "Grounding stats:\n\t* " << grounded.size() << " grounded elements\n\t* " << total_num_bindings - grounded.size() << " pruned elements");
	LPT_DEBUG("grounding", "All ground actions " << std::endl << print::actions(grounded));
	return grounded;
}

std::vector<const GroundAction*>
ActionGrounder::fully_ground(const std::vector<const ActionData*>& action_data, const ProblemInfo& info) {
	std::vector<const GroundAction*> grounded = _loadGroundActionsIfAvailable(info, action_data);
	if (!grounded.empty()) { // A previous grounding was found, return it
		return grounded;
	}

	return _ground_all_elements(action_data, info, true);
}


std::vector<const PartiallyGroundedAction*>
ActionGrounder::compile_action_parameters_away(const PartiallyGroundedAction* schema, unsigned effect_idx, const ProblemInfo& info) {
	const fs::ActionEffect* effect = schema->getEffects().at(effect_idx);

	auto head_parameters = Utils::filter_by_type<const fs::BoundVariable*>(fs::all_nodes(*effect->lhs()));

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

		if (PartiallyGroundedAction* grounded_action = _partial_binding(schema->getActionData(), binding, info)) {
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
	for (const object_id&
 		value:info.getTypeObjects(fs::type(*subterm_to_replace))) {
		auto subterms = Utils::clone(original_subterms);
		delete subterms[i];

		subterms[i] = new fs::Constant(value, UNSPECIFIED_NUMERIC_TYPE);
		auto extra_condition = new fs::EQAtomicFormula({subterm_to_replace->clone(), subterms[i]->clone()});
		auto new_condition = fs::conjunction(*effect->condition(), *extra_condition);
		delete extra_condition;

		auto tmp_head = new fs::FluentHeadedNestedTerm(head->getSymbolId(), subterms);
		auto processed_head = fs::bind(*tmp_head, {}, info);
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
		for (const object_id&
 value:info.getTypeObjects(subterm->getType())) {
			auto constant = new fs::Constant(value);
// 			std::vector<const fs::Term*> subterms{subterm->clone(), new fs::Constant(value)};
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
	for (const object_id&
 value:info.getTypeObjects(subterm->getType())) {

		std::vector<const fs::Term*> subterms{subterm->clone(), new fs::Constant(value)};
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


ActionData*
ActionGrounder::process_action_data(const ActionData& action, const ProblemInfo& info, bool process_effects) {
	auto precondition = fs::bind(*action.getPrecondition(), Binding::EMPTY_BINDING, info);
	if (precondition->is_contradiction()) {
		delete precondition;
        LPT_INFO("cout", "Warning: action schema " << action.getName() << " discarded because precondition is (statically) unsatisfiable");
        return nullptr;
    }

	std::vector<const fs::ActionEffect*> effects;

	if (process_effects) {
		effects = _bind_effects(action, Binding::EMPTY_BINDING, info);
		if (effects.empty() && !action.hasProceduralEffects()) {
			delete precondition;
            LPT_INFO("cout", "Warning: action schema " << action.getName() << " discarded because it has no applicable effects");
            return nullptr;
		}
	}
	return new ActionData(action.getId(), action.getName(), action.getSignature(), action.getParameterNames(), action.getBindingUnit(), precondition, effects, action.getType());
}

GroundAction*
ActionGrounder::bind(const PartiallyGroundedAction& action, const Binding& binding, const ProblemInfo& info) {
	Binding full(action.getBinding());
	full.merge_with(binding); // TODO We should bind not from the action data but from the partially bound action itself.
	return _full_binding(GroundAction::invalid_action_id, action.getActionData(), full, info, true);
}


} // namespaces
