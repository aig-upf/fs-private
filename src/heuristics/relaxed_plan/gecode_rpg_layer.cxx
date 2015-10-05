

#include <heuristics/relaxed_plan/gecode_rpg_layer.hxx>
#include <problem.hxx>
#include <relaxed_state.hxx>

namespace fs0 {

GecodeRPGLayer::GecodeRPGLayer(const std::vector<std::vector<int>>& domains) {
	for (VariableIdx variable = 0; variable < domains.size(); ++variable) {
		const auto& domain = domains[variable];
		_domains.push_back(Gecode::IntSet(domain.data(), domain.size()));
	}
}

GecodeRPGLayer::GecodeRPGLayer(const RelaxedState& layer) {
	auto layer_domains = layer.getDomains();
	for (auto layer_domain:layer_domains) {
		// An intermediate IntArgs object seems to be necessary, since IntSets do not accept std-like range constructors.
		_domains.push_back(Gecode::IntSet(Gecode::IntArgs(layer_domain->cbegin(), layer_domain->cend())));
	}
}


std::ostream& GecodeRPGLayer::print(std::ostream& os) const {
	const ProblemInfo& problemInfo = Problem::getInfo();
	os << "GecodeRPGLayer[";
	for (unsigned i = 0; i < _domains.size(); ++i) { // Iterate through all the sets
		os << problemInfo.getVariableName(i) << "={";
		for (Gecode::IntSetValues values(_domains.at(i)); values(); ++values) {
			os << problemInfo.getObjectName(i, values.val()) << ",";
		}
		os << "}";
		if (i < _domains.size() - 1) os << ", ";
	}
	os << "]";
	return os;
}

} // namespaces
