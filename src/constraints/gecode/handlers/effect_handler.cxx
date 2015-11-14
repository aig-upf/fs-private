
#include <constraints/gecode/handlers/effect_handler.hxx>
#include <actions/ground_action.hxx>

namespace fs0 { namespace gecode {

GecodeEffectCSPHandler::GecodeEffectCSPHandler(const GroundAction& action, unsigned effect_idx, bool use_novelty_constraint)
	: GecodeActionCSPHandler(action, {action.getEffects().at(effect_idx)}, use_novelty_constraint) {}

} } // namespaces
