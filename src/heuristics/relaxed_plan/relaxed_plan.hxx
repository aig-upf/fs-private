
#pragma once

namespace fs0 { class TupleIndex; }
namespace fs0 { namespace gecode { class RPGIndex; class FormulaCSP; }}

namespace fs0 { namespace gecode { namespace support {

//! Compute the length of a relaxed plan, if exists, or -1 if not
long compute_rpg_cost(const TupleIndex& tuple_index, const RPGIndex& graph, const FormulaCSP& goal_handler);

long compute_hmax_cost(const TupleIndex& tuple_index, const RPGIndex& graph, const FormulaCSP& goal_handler);

} } } // namespaces
