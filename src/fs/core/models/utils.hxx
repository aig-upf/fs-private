
#pragma once

#include <vector>

namespace fs0::language::fstrips { class Formula; }

namespace fs = fs0::language::fstrips;

std::vector<const fs::Formula*> obtain_goal_atoms(const fs::Formula* goal);