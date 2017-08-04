
#include <unordered_set>

#include <lapkt/tools/logging.hxx>

#include <problem_info.hxx>
#include <constraints/grounding.hxx>
#include <languages/fstrips/axioms.hxx>
#include <utils/printers/binding.hxx>
#include <utils/printers/language.hxx>
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
    	const fs::Formula* definition = fs::bind(*lifted.getDefinition(), binding, info);
    	if (definition->is_contradiction()) {
    		delete definition;
    		return nullptr;
    	}
    	return new fs::Axiom(lifted.getName(), lifted.getSignature(), lifted.getParameterNames(), lifted.getBindingUnit(), definition);
    }

    //! Grounds the set of given action schemata with all parameter groundings that induce no false preconditions
    //! Returns the new set of grounded actions
    unsigned
    _ground(unsigned id, const fs::Axiom* data, const Binding& binding, const ProblemInfo& info, std::vector<const fs::Axiom*>& grounded) {
    // 	LPT_DEBUG("grounding", "Binding: " << print::binding(binding, data->getSignature()));

    	if (fs::Axiom* ground = _full_binding( *data, binding, info)) {
    		LPT_EDEBUG("groundings", "\t" << *ground);
    		grounded.push_back(ground);
    		return id + 1;
    	} else {
    		LPT_DEBUG("grounding", "Binding " << print::binding(binding, data->getSignature()) << " generates a statically non-applicable grounded axiom");
    	}
    	return id;
    }


    std::vector< const fs::Axiom* >
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
                id = _ground(id, data, Binding::EMPTY_BINDING, info, grounded);
                ++total_num_bindings;
                continue;
            }

            utils::binding_iterator binding_generator(signature, info);
            if (binding_generator.ended()) {
                LPT_INFO("cout", "Grounding of schema '" << data->getName() << "' yields no ground element, likely due to a parameter with empty type");
                continue;
            }

            unsigned long num_bindings = binding_generator.num_bindings();

            LPT_INFO("cout", "Grounding schema '" << print::axiom_header(*data) << "' with " << num_bindings << " possible bindings" << std::flush);
            LPT_INFO("grounding", "Grounding the following schema with " << num_bindings << " possible bindings:" << print::axiom_header(*data));

            if (num_bindings == 0 || num_bindings > AxiomGrounder::MAX_GROUND_AXIOMS) { // num_bindings == 0 would indicate there's been an overflow
                //throw TooManyfs::AxiomsError(num_bindings);
                LPT_INFO("cout", "WARNING - The number of ground elements is too high: " << num_bindings);
            }

    // 		float onepercent = ((float)num_bindings / 100);
    // 		int progress = 0;
    // 		unsigned i = 0;
            for (; !binding_generator.ended(); ++binding_generator) {
                id = _ground(id, data, *binding_generator, info, grounded);
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
            LPT_INFO("cout", "Schema \"" << print::axiom_header(*data) << "\" results in " << grounded.size() - grounded_0 << " grounded elements");
        }

        LPT_INFO("grounding", "Grounding stats:\n\t* " << grounded.size() << " grounded elements\n\t* " << total_num_bindings - grounded.size() << " pruned elements");
        LPT_INFO("cout", "Grounding stats:\n\t* " << grounded.size() << " grounded elements\n\t* " << total_num_bindings - grounded.size() << " pruned elements");
        return grounded;

    }

}
