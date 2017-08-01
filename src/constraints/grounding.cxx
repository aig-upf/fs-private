
#include <unordered_set>

#include <lapkt/tools/logging.hxx>

#include <problem_info.hxx>
#include <constraints/grounding.hxx>
#include <languages/fstrips/actions.hxx>
#include <utils/printers/binding.hxx>
#include <utils/config.hxx>
#include <utils/binding_iterator.hxx>
#include <utils/utils.hxx>
#include <utils/loader.hxx>
#include <languages/fstrips/language.hxx>
#include <languages/fstrips/operations.hxx>

namespace fs0 {

    fs::Axiom*
    _full_binding( const fs::Axiom& lifted, const Binding& binding, const ProblemInfo& info) {
    	assert(binding.is_complete()); // Grounding only possible for full bindings
    	const fs::Formula* definition = fs::bind(*action_data.getDefinition(), binding, info);
    	if (precondition->is_contradiction()) {
    		delete precondition;
    		return nullptr;
    	}
    	return new Axiom(lifted.getName(), lifted.getSignature(), {}, lifted.getBindingUnit(), definition);
    }

    std::vector< fs::Axiom* >
    AxiomGrounder::fully_ground( const std::vector<const fs::Axiom*>& schemata, const ProblemInfo& info ) {
        std::vector<const fs::Axiom*> grounded;

        unsigned total_num_bindings = 0;

        unsigned id = 0;
        for (const fs::Axiom* data : schemata) {
            unsigned grounded_0 = grounded.size();
            const Signature& signature = data->getSignature();

            // In case the action schema is directly not-lifted, we simply bind it with an empty binding and continue.
            if (signature.empty()) {
                LPT_INFO("cout", "Grounding schema '" << data->getName() << "' with no binding");
                LPT_INFO("grounding", "Grounding the following schema with no binding:" << *data << "\n");
                id = _ground(data, Binding::EMPTY_BINDING, info, grounded);
                ++total_num_bindings;
                continue;
            }

            utils::binding_iterator binding_generator(signature, info);
            if (binding_generator.ended()) {
                LPT_INFO("cout", "Grounding of schema '" << data->getName() << "' yields no ground element, likely due to a parameter with empty type");
                continue;
            }

            unsigned long num_bindings = binding_generator.num_bindings();

            LPT_INFO("cout", "Grounding schema '" << print::action_data_name(*data) << "' with " << num_bindings << " possible bindings" << std::flush);
            LPT_INFO("grounding", "Grounding the following schema with " << num_bindings << " possible bindings:" << print::action_data_name(*data));

            if (num_bindings == 0 || num_bindings > ActionGrounder::MAX_GROUND_ACTIONS) { // num_bindings == 0 would indicate there's been an overflow
                //throw TooManyfs::AxiomsError(num_bindings);
                LPT_INFO("cout", "WARNING - The number of ground elements is too high: " << num_bindings);
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
            LPT_INFO("cout", "Schema \"" << print::action_data_name(*data) << "\" results in " << grounded.size() - grounded_0 << " grounded elements");
        }

        LPT_INFO("grounding", "Grounding stats:\n\t* " << grounded.size() << " grounded elements\n\t* " << total_num_bindings - grounded.size() << " pruned elements");
        LPT_INFO("cout", "Grounding stats:\n\t* " << grounded.size() << " grounded elements\n\t* " << total_num_bindings - grounded.size() << " pruned elements");
        LPT_DEBUG("grounding", "All ground actions " << std::endl << print::actions(grounded));
        return grounded;

    }

}
