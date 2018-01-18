
#include <fs/core/fs_types.hxx>
#include <fs/core/utils/printers/helper.hxx>
#include <fs/core/problem_info.hxx>


namespace fs0 {

UnindexedAtom::UnindexedAtom(VariableIdx variable, const object_id& value) :
        std::runtime_error(printer() << "Unindexed atom " << ProblemInfo::getInstance().getVariableName(variable) << "=" << value)
{}

} // namespaces
