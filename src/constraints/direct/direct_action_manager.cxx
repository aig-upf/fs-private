
#include <cassert>
#include <iosfwd>

#include <constraints/direct/direct_action_manager.hxx>
#include <heuristics/rpg_data.hxx>
#include <constraints/direct/translators/translator.hxx>
#include <constraints/direct/bound_constraint.hxx>
#include <utils/projections.hxx>
#include <utils/logging.hxx>

namespace fs0 {

DirectActionManager* DirectActionManager::create(const GroundAction& action) {
	std::vector<DirectConstraint::cptr> constraints = DirectTranslator::generate(action.getConditions());
	std::vector<DirectEffect::cptr> effects = DirectTranslator::generate(action.getEffects());
	
	// Add the necessary bound-constraints
	BoundsConstraintsGenerator::generate(action, effects, constraints);
	
	// Compile constraints if necessary
	ConstraintCompiler::compileConstraints(constraints);
	
	return new DirectActionManager(action, std::move(constraints), std::move(effects));
}

DirectActionManager::DirectActionManager(const GroundAction& action, std::vector<DirectConstraint::cptr>&& constraints, std::vector<DirectEffect::cptr>&& effects)
	: BaseActionManager(),
	  _action(action),
	  _constraints(constraints),
	  _effects(effects),
	  _handler(_constraints)
{}

DirectActionManager::~DirectActionManager() {
	for (const auto ptr:_constraints) delete ptr;
	for (const auto ptr:_effects) delete ptr;
	
}

void DirectActionManager::process(unsigned actionIdx, const RelaxedState& layer, RPGData& rpg) {
	// We compute the projection of the current relaxed state to the variables relevant to the action
	// Note that this _clones_ the actual domains, since we will next modify (prune) them.
	DomainMap actionProjection = Projections::projectCopyToActionVariables(layer, _action);

	if (checkPreconditionApplicability(actionProjection)) { // Check with local consistency
		processEffects(actionIdx, actionProjection, rpg);
	}
}

bool DirectActionManager::checkPreconditionApplicability(const DomainMap& domains) const {
	FilteringOutput o = _handler.filter(domains);
	return o != FilteringOutput::Failure && DirectCSPHandler::checkConsistency(domains);
}

void DirectActionManager::processEffects(unsigned actionIdx, const DomainMap& actionProjection, RPGData& rpg) const {
	const VariableIdxVector& actionScope = _action.getScope();
	FFDEBUG("main", "processing action effects: " << _action.getName());

	for (const DirectEffect::cptr effect:_effects) {
		const VariableIdxVector& effectScope = effect->getScope();

		/***** 0-ary Effects *****/
		if(effectScope.size() == 0) {  // No need to pass any point.
			assert(effect->applicable()); // The effect is assumed to be applicable - non-applicable 0-ary effects make no sense and are detected before the search.
			Atom atom = effect->apply();
			auto hint = rpg.getInsertionHint(atom);

			if (hint.first) {
				Atom::vctrp atomSupport = std::make_shared<Atom::vctr>(); // 0-ary effects will have no atom support
				Atom::vctrp actionSupport = std::make_shared<Atom::vctr>();
				completeAtomSupport(actionScope, actionProjection, effectScope, actionSupport);
				rpg.add(atom, actionIdx, actionSupport, atomSupport, hint.second);
			}
		}

		/***** Unary Effects *****/
		else if(effectScope.size() == 1) {  // Micro-optimization for unary effects
			for (ObjectIdx value:*(actionProjection.at(effectScope[0]))) { // Add to the RPG for every allowed value of the relevant variable
				if (!effect->applicable(value)) continue;
				Atom atom = effect->apply(value);
				auto hint = rpg.getInsertionHint(atom);

				if (hint.first) {
				Atom::vctrp atomSupport = std::make_shared<Atom::vctr>();
				Atom::vctrp actionSupport = std::make_shared<Atom::vctr>();
					atomSupport->push_back(Atom(effectScope[0], value));// Just insert the only value
					completeAtomSupport(actionScope, actionProjection, effectScope, actionSupport);
					rpg.add(atom, actionIdx, actionSupport, atomSupport, hint.second);
				}
			}
		}

		/***** Higher-arity Effects: Only supported with the gecode handler *****/
		else throw std::runtime_error("Shouldn't be here!");
	}
}

void DirectActionManager::completeAtomSupport(const VariableIdxVector& actionScope, const DomainMap& actionProjection, const VariableIdxVector& effectScope, Atom::vctrp support) const {
	for (VariableIdx variable:actionScope) {
		if (effectScope.empty() || variable != effectScope[0]) { // (We know that the effect scope has at most one variable)
			ObjectIdx value = *(actionProjection.at(variable)->cbegin());
			support->push_back(Atom(variable, value));
		}
	}
}


std::ostream& DirectActionManager::print(std::ostream& os) const {
	os << "DirectActionManager[]";
	return os;
}


} // namespaces
