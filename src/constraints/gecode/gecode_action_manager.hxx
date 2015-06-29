#ifndef __GECODE_ACTION_MANAGER__
#define __GECODE_ACTION_MANAGER__

#include <relaxed_action_manager.hxx>
#include <constraints/gecode/support_csp.hxx>

namespace fs0 {


  namespace gecode {

    class RelaxedActionManager : public BaseActionManager {
    public:

      RelaxedActionManager( const Action& action, bool hasNaryEffects );
      virtual ~RelaxedActionManager();

      virtual void processAction(unsigned actionIdx, const Action& action, const RelaxedState& layer, RPGData& changeset) const;


      static  SupportCSP::vptr      CSPSet;

    };


  }

}

#endif // gecode_action_manager.hxx
