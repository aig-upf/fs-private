
#include <utils/binding_iterator.hxx>
#include <problem_info.hxx>

namespace fs0 { namespace utils {

const ObjectIdxVector binding_iterator::NIL{0};


binding_iterator::binding_iterator(const std::vector<TypeIdx>& types, const ProblemInfo& info) :
	_valid(), // To avoid two iterations, this will get initialized in the call to 'generate_values' below
	_iterator(generate_values(types, info))
{}

std::vector<const ObjectIdxVector*> binding_iterator::generate_values(const std::vector<TypeIdx>& types, const ProblemInfo& info) {
	std::vector<const ObjectIdxVector*> values;
	
	for (TypeIdx type:types) {
		if (type == INVALID_TYPE) {
			values.push_back(&NIL);
			_valid.push_back(false);
		} else {
			values.push_back(&info.getTypeObjects(type));
			_valid.push_back(true);
		}
	}
	
	return values;
}

} } // namespaces
