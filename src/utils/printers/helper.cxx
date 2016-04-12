
#include <utils/printers/helper.hxx>
#include <problem_info.hxx>

namespace fs0 { namespace print {

const std::vector<std::string> Helper::name_variables(const std::vector<VariableIdx>& variables) {
	const ProblemInfo& info = ProblemInfo::getInstance();
	std::vector<std::string> names;
	for (VariableIdx variable:variables) {
		names.push_back(info.getVariableName(variable));
	}
	return names;
}

} } // namespaces
