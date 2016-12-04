
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

const std::vector<std::string> Helper::name_objects(const std::vector<ObjectIdx>& objects, const Signature& signature) {
	assert(objects.size() == signature.size());
	const ProblemInfo& info = ProblemInfo::getInstance();
	std::vector<std::string> names;
	for (unsigned i = 0; i < objects.size(); ++i) {
		names.push_back(info.deduceObjectName(objects[i], signature[i]));
	}
	return names;
}

} } // namespaces
