
#include <languages/fstrips/language.hxx>
#include <constraints/gecode/handlers/ground_effect_handler.hxx>
#include <actions/ground_action.hxx>
#include <utils/printers/actions.hxx>
#include <utils/logging.hxx>
#include <actions/action_id.hxx>

namespace fs0 { namespace gecode {

std::vector<std::shared_ptr<BaseActionCSPHandler>> GroundEffectCSPHandler::create(const std::vector<const GroundAction*>& actions, bool approximate, bool novelty) {
	std::vector<std::shared_ptr<BaseActionCSPHandler>> managers;
	
	for (unsigned action_idx = 0; action_idx < actions.size(); ++action_idx) {
		const auto action = actions[action_idx];
		
		for (unsigned eff_idx = 0; eff_idx < action->getEffects().size(); ++eff_idx) {
			auto handler = std::make_shared<GroundEffectCSPHandler>(*action, eff_idx, approximate, novelty);
			managers.push_back(handler);
			FDEBUG("main", "Generated CSP for the effect #" << eff_idx << " of action " << print::action_name(*action) << std::endl <<  *handler << std::endl);
		}
	}
	return managers;
}

GroundEffectCSPHandler::GroundEffectCSPHandler(const GroundAction& action, unsigned effect_idx, bool approximate, bool novelty)
	: BaseActionCSPHandler(action, { action.getEffects().at(effect_idx) }, approximate, novelty)
{}

void GroundEffectCSPHandler::log() const {
	assert(_effects.size() == 1);
	FFDEBUG("heuristic", "Processing effect \"" << *_effects.at(0) << " of action " << _action.fullname());
}

const ActionID* GroundEffectCSPHandler::get_action_id(SimpleCSP* solution) const {
	return new PlainActionID(_action.getId());
}

} } // namespaces
