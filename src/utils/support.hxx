
#pragma once

#include <vector>

namespace fs0 { class ActionBase; }

namespace fs0 { namespace language { namespace fstrips { class Formula; }}}
namespace fs = fs0::language::fstrips;


namespace fs0 { namespace support {

//! Return all those symbols whose extensions need to be tracked down
std::vector<bool> compute_managed_symbols(const std::vector<const ActionBase*>& actions, const fs::Formula* goal_formula, const fs::Formula* state_constraints);

} } // namespaces
