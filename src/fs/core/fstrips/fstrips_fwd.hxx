
#pragma once

//! Forward declarations of the most relevant elements in the FSTRIPS namespace

#include "fol_syntax_fwd.hxx"  // Forward-declares most relevant syntactic elements

namespace fs0::fstrips {

using symbol_id = unsigned;
enum class symbol_t {Predicate, Function};

//! FSTRIPS action language elements
class AtomicEffect;
class FunctionalEffect;
class ActionSchema;
enum class AtomicEffectType;

class FSType;
class SymbolInfo;
class LanguageInfo;

} // namespaces

