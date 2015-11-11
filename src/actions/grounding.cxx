
#include <actions/grounding.hxx>
#include <problem.hxx>
#include <utils/logging.hxx>
#include <utils/cartesian_iterator.hxx>
#include <utils/printers/binding.hxx>

namespace fs0 {

std::vector<GroundAction::cptr> ActionGrounder::ground(const std::vector<ActionSchema::cptr>& schemata, const ProblemInfo& info) {
	
	std::vector<GroundAction::cptr> grounded;
	unsigned total_num_bindings = 0;
	
	for (const ActionSchema::cptr schema:schemata) {
		const Signature& signature = schema->getSignature();
		
		// In case the action schema is directly not-lifted, we simply bind it with an empty binding and continue.
		if (signature.empty()) { 
			std::cout <<  "Grounding action schema '" << schema->getName() << "' with no binding" << std::endl;
			FINFO("grounding", "Grounding the following action schema with no binding:\n" << *schema << "\n");
			ground(schema, {}, info, grounded);
			++total_num_bindings;
			continue;
		}
		
		
		std::vector<const ObjectIdxVector*> values = info.getSignatureValues(signature);
		int num_bindings = std::accumulate(values.begin(), values.end(), 1, [](int a, const ObjectIdxVector* b) { return a * b->size(); });
		
		std::cout <<  "Grounding action schema '" << schema->getName() << "' with " << num_bindings << " possible bindings:" << std::endl << "\t" << std::flush;
		FINFO("grounding", "Grounding the following action schema with " << num_bindings << " possible bindings:\n" << *schema << "\n");
		
		float onepercent = ((float)num_bindings / 100);
		int progress = 0;
		
		utils::cartesian_iterator cross_product(std::move(values));
		unsigned i = 0;
		for (; !cross_product.ended(); ++cross_product) {
			ground(schema, Binding(*cross_product), info, grounded);
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

void ActionGrounder::ground(ActionSchema::cptr schema, const Binding& binding, const ProblemInfo& info, std::vector<GroundAction::cptr>& grounded) {
	FDEBUG("grounding", "Binding: " << print::binding(binding, schema->getSignature()));
	GroundAction* ground = schema->bind(binding, info);
	if (ground) {
		FDEBUG("grounding", "Binding " << print::binding(binding, schema->getSignature()) << " generated grounded action:\n" << *ground);
		grounded.push_back(ground);
	} else {
		FDEBUG("grounding", "Binding " << print::binding(binding, schema->getSignature()) << " generates a statically non-applicable grounded action");
	}	
}

} // namespaces
