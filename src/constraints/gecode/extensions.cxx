

#include <constraints/gecode/extensions.hxx>
#include <problem.hxx>
#include <state.hxx>

namespace fs0 { namespace gecode {

ExtensionHandler::ExtensionHandler() :
	_info(Problem::getInfo())
{}

void ExtensionHandler::reset() {
	_extensions = std::vector<Extension>(_info.getNumLogicalSymbols()); // Reset the whole vector
	advance();
}

void ExtensionHandler::advance() {
	_modified.clear(); // Initially all symbols are untouched
}

void ExtensionHandler::process_atom(VariableIdx variable, ObjectIdx value) {
	process_delta(variable, {value});
}

void ExtensionHandler::process_delta(VariableIdx variable, const std::vector<ObjectIdx>& delta) {
	const auto& vardata = _info.getVariableData(variable);
	bool is_predicate = _info.isPredicate(vardata.first);
	Extension& extension = _extensions.at(vardata.first);
	_modified.insert(vardata.first);  // Mark the extension as modified
	
	for (ObjectIdx value:delta) {
		if (is_predicate && value == 1) {
			extension.add_tuple(vardata.second);
		}
		
		else if (!is_predicate) {
			std::vector<ObjectIdx> tuple(vardata.second); // (implicitly copies)
			tuple.push_back(value);
			extension.add_tuple(tuple);
		}
	}
}

std::vector<Gecode::TupleSet> ExtensionHandler::generate_extensions() const {
	std::vector<Gecode::TupleSet> result;
	for (auto& generator:_extensions) {
		result.push_back(generator.generate());
	}
	return result;
}

} } // namespaces
