
#include "light_operations.hxx"
#include <problem_info.hxx>



namespace fs0 { namespace lang { namespace fstrips {


bool has_empty_parameter(const ActionSchema& schema) {
	const ProblemInfo& info = ProblemInfo::getInstance();
	for (TypeIdx type:schema.getSignature()) {
		if (info.getTypeObjects(type).empty()) return true;
	}
	return false;
}

} } } // namespaces
