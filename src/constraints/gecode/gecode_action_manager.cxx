#include <constraints/gecode/gecode_action_manager.hxx>
#include <utils/projections.cxx>
#include <tuple>

namespace fs0 {

  namespace gecode {

    SupportCSP::vptr  RelaxedActionManager::CSPSet;

    RelaxedActionManager::RelaxedActionManager( const Action& action, bool hasNaryEffects ) :
      BaseActionManager(action, hasNaryEffects) {

    }

    RelaxedActionManager::~RelaxedActionManager() {

    }

    void
    RelaxedActionManager::processAction(  unsigned actionIdx, const Action& action,
                                          const RelaxedState& layer, RPGData& changeset) const {
      SupportCSP::ptr tmp = CSPSet[actionIdx]->copy(true);

      // Setup domain constraints etc.
      DomainMap actionProjection = Projections::projectToActionVariables(layer, action);
      for ( auto entry : actionProjection ) {
        VariableIdx x;
        DomainPtr dom;
        std::tie( x, dom ) = entry;
        if ( dom.size() == 1 ) {
          tmp->addEqualityConstraint( x, *(dom->begin()) );
          continue;
        }
        ObjectIdx lb = *(dom->begin());
        ObjectIdx ub = *(dom->rbegin());
        // MRJ: Check this is a safe assumption
        if ( dom.size() == (ub - lb) ) {
          tmp->addBoundsConstraint( x, lb, ub );
          continue;
        }
        // MRJ: worst case (performance wise) yet I think it can be optimised in
        // a number of ways
        tmp->addMembershipConstraint( x, dom );
      }

      // Check local consistency
      bool consistent = tmp->locallyConsistent();
      if (!consistent) return; // We're done

      for ( ScopedEffect::cptr effect : action.getEffects() ) {
        // MRJ: What are exactly supports?

        Atom::vctrp support = std::make_shared<Atom::vctr>();

        rpgData.add(effect->apply(values), actionIdx, support);
      }

    }

  }

}
