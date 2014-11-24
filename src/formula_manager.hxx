
#ifndef __APTK_CORE_FORMULA_MANAGER_HXX__
#define __APTK_CORE_FORMULA_MANAGER_HXX__

#include <cassert>
#include <iosfwd>
#include <fact.hxx>



namespace aptk { namespace core {
	
	
/**
 */
class FormulaManager
{
public:
	static bool isApplicable(const ApplicableEntity& action, const State& state);
};

} } // namespaces

#endif
