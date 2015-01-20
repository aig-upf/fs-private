
#include <constraints/scoped_effect.hxx>
#include <utils/projections.hxx>

namespace fs0 {

ScopedEffect::ScopedEffect(const VariableIdxVector& scope, const VariableIdxVector& image, const std::vector<int>& parameters) :
	_scope(scope), _affected(image[0]), _binding(parameters) {
		if (image.size() != 1) {
			throw std::runtime_error("We currently only handle action effects that affect a single variable");
		}
	}
	
} // namespaces

