
#include <constraints/direct/action_manager.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <constraints/direct/translators/translator.hxx>
#include <constraints/direct/bound_constraint.hxx>
#include <utils/projections.hxx>
#include <utils/logging.hxx>
#include <languages/fstrips/scopes.hxx>
#include <relaxed_state.hxx>
#include <actions/action_id.hxx>

namespace fs0 {

std::vector<std::shared_ptr<DirectActionManager>> DirectActionManager::create(const std::vector<GroundAction::cptr>& actions) {
	std::vector<std::shared_ptr<DirectActionManager>> managers;
	managers.reserve(actions.size());
	for (const auto action:actions) {
		auto manager = create(*action);
		FDEBUG("main", "Generated a DirectActionManager for action " << *action << std::endl << *manager << std::endl);
		managers.push_back(std::move(manager));
	}
	return managers;
}

std::shared_ptr<DirectActionManager> DirectActionManager::create(const GroundAction& action) {
	assert(is_supported(action));
	
	std::vector<DirectConstraint::cptr> constraints;
	
	if (!dynamic_cast<fs::Tautology::cptr>(action.getPrecondition())) { // If the precondition is a tautology, we'll have no constraints
		auto precondition = dynamic_cast<fs::Conjunction::cptr>(action.getPrecondition());
		assert(precondition);
		constraints = DirectTranslator::generate(precondition->getConjuncts());
	}
	
	std::vector<DirectEffect::cptr> effects = DirectTranslator::generate(action.getEffects());
	
	// Add the necessary bound-constraints
	BoundsConstraintsGenerator::generate(action, effects, constraints);
	
	// Compile constraints if necessary
	ConstraintCompiler::compileConstraints(constraints);
	
	return std::make_shared<DirectActionManager>(action, std::move(constraints), std::move(effects));
}

bool DirectActionManager::is_supported(const GroundAction& action) {
	if (dynamic_cast<fs::Tautology::cptr>(action.getPrecondition())) return true;
	auto precondition = dynamic_cast<fs::Conjunction::cptr>(action.getPrecondition());
	
	// Only conjunctions of atoms are supported by the direct translator
	if (!precondition) return false; 
	
	for (const fs::AtomicFormula::cptr condition:precondition->getConjuncts()) {
		if (condition->nestedness() > 0) return false;
		
		unsigned arity = fs::ScopeUtils::computeDirectScope(condition).size();
		if (arity == 0) throw std::runtime_error("Static applicability procedure that should have been detected in compilation time");
		else if(arity > 1) return false;
	}
	
	for (const fs::ActionEffect::cptr effect:action.getEffects()) {
		if (effect->lhs()->nestedness() > 0 || effect->rhs()->nestedness() > 0 || fs::ScopeUtils::computeDirectScope(effect).size() > 1) return false;
	}
	
	return true;
}


DirectActionManager::DirectActionManager(const GroundAction& action, std::vector<DirectConstraint::cptr>&& constraints, std::vector<DirectEffect::cptr>&& effects)
	: 
	  _action(action),
	  _constraints(constraints),
	  _effects(effects),
	  _scope(fs::ScopeUtils::computeActionDirectScope(action)),
	  _allRelevant(extractAllRelevant()),
	  _handler(_constraints)
{}

DirectActionManager::~DirectActionManager() {
	for (const auto ptr:_constraints) delete ptr;
	for (const auto ptr:_effects) delete ptr;
	
}

VariableIdxVector DirectActionManager::extractAllRelevant() const {
	std::set<VariableIdx> unique(_scope.begin(), _scope.end());
	for (fs::ActionEffect::cptr effect:_action.getEffects()) fs::ScopeUtils::computeDirectScope(effect, unique);
	return VariableIdxVector(unique.cbegin(), unique.cend());
}

void DirectActionManager::process(unsigned int actionIdx, const fs0::RelaxedState& layer, fs0::RPGData& rpg) const {
	// We compute the projection of the current relaxed state to the variables relevant to the action
	// Note that this _clones_ the actual domains, since we will next modify (prune) them.
	DomainMap actionProjection = Projections::projectCopy(layer, _allRelevant);
	
	if (checkPreconditionApplicability(actionProjection)) { // Check with local consistency
		processEffects(actionIdx, actionProjection, rpg);
	}
}


bool DirectActionManager::checkPreconditionApplicability(const DomainMap& domains) const {
	FilteringOutput o = _handler.filter(domains);
	return o != FilteringOutput::Failure && DirectCSPHandler::checkConsistency(domains);
}

void DirectActionManager::processEffects(unsigned actionIdx, const DomainMap& actionProjection, RPGData& rpg) const {
	for (const DirectEffect::cptr effect:_effects) {
		const VariableIdxVector& effectScope = effect->getScope();

		/***** 0-ary Effects *****/
		if(effectScope.size() == 0) {  // No need to pass any point.
			assert(effect->applicable()); // The effect is assumed to be applicable - non-applicable 0-ary effects make no sense and are detected before the search.
			Atom atom = effect->apply();
			auto hint = rpg.getInsertionHint(atom);

			if (hint.first) {
				FFDEBUG("heuristic", "Processing effect \"" << *effect << "\" yields " << (hint.first ? "new" : "repeated") << " atom " << atom);
				Atom::vctrp support = std::make_shared<Atom::vctr>();
				completeAtomSupport(_scope, actionProjection, effectScope, support);
				rpg.add(atom, get_action_id(actionIdx), support, hint.second);
			}
		}

		/***** Unary Effects *****/
		else if(effectScope.size() == 1) {  // Micro-optimization for unary effects
			for (ObjectIdx value:*(actionProjection.at(effectScope[0]))) { // Add to the RPG for every allowed value of the relevant variable
				if (!effect->applicable(value)) continue;
				Atom atom = effect->apply(value);
				auto hint = rpg.getInsertionHint(atom);

				if (hint.first) {
					FFDEBUG("heuristic", "Processing effect \"" << *effect << "\" yields " << (hint.first ? "new" : "repeated") << " atom " << atom);
					Atom::vctrp support = std::make_shared<Atom::vctr>();
					support->push_back(Atom(effectScope[0], value));// Just insert the only value
					completeAtomSupport(_scope, actionProjection, effectScope, support);
					rpg.add(atom, get_action_id(actionIdx), support, hint.second);
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

const ActionID* DirectActionManager::get_action_id(unsigned action_idx) const {
	return new PlainActionID(action_idx);
}

} // namespaces
