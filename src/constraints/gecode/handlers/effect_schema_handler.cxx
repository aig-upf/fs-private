
#include <constraints/gecode/handlers/effect_schema_handler.hxx>
#include <utils/printers/actions.hxx>
#include <utils/logging.hxx>

namespace fs0 { namespace gecode {

std::vector<std::shared_ptr<BaseActionCSPHandler>> EffectSchemaCSPHandler::create(const std::vector<const ActionSchema*>& schemata, bool approximate, bool novelty, bool dont_care) {
	// Simply upcast the shared_ptrs
	std::vector<std::shared_ptr<BaseActionCSPHandler>> handlers;
	for (const auto& element:create_derived(schemata, approximate, novelty, dont_care)) {
		handlers.push_back(std::static_pointer_cast<BaseActionCSPHandler>(element));
	}
	return handlers;
}

std::vector<std::shared_ptr<EffectSchemaCSPHandler>> EffectSchemaCSPHandler::create_derived(const std::vector<const ActionSchema*>& schemata, bool approximate, bool novelty, bool dont_care) {
	std::vector<std::shared_ptr<EffectSchemaCSPHandler>> handlers;
	
	for (auto schema:schemata) {
		for (unsigned eff_idx = 0; eff_idx < schema->getEffects().size(); ++eff_idx) {
			auto handler = std::make_shared<EffectSchemaCSPHandler>(*schema, eff_idx, approximate, novelty, dont_care);
			FDEBUG("main", "Generated CSP for the effect #" << eff_idx << " of action " << schema->fullname() << std::endl <<  *handler << std::endl);
			handlers.push_back(handler);
		}
	}
	return handlers;
}

EffectSchemaCSPHandler::EffectSchemaCSPHandler(const ActionSchema& action, unsigned effect_idx, bool approximate, bool novelty, bool dont_care)
	: ActionSchemaCSPHandler(action, { action.getEffects().at(effect_idx) }, approximate, novelty, dont_care)
{}

void EffectSchemaCSPHandler::log() const {
	assert(_effects.size() == 1);
	FFDEBUG("heuristic", "Processing effect schema \"" << *_effects.at(0) << " of action " << _action.fullname());
}

} } // namespaces
