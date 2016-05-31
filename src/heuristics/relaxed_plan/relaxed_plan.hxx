
#pragma once

#include <fs_types.hxx>

namespace fs0 { class TupleIndex; class Atom; }
namespace fs0 { namespace gecode { class RPGIndex; class FormulaCSP; }}

namespace fs0 { namespace gecode { namespace support {

//! Returns the length of a relaxed plan, if exists, or -1 otherwise
long compute_rpg_cost(const TupleIndex& tuple_index, const RPGIndex& graph, const FormulaCSP& goal_handler, std::vector<Atom>& relevant);
long compute_rpg_cost(const TupleIndex& tuple_index, const RPGIndex& graph, const FormulaCSP& goal_handler);

long compute_hmax_cost(const TupleIndex& tuple_index, const RPGIndex& graph, const FormulaCSP& goal_handler);

} } } // namespaces
