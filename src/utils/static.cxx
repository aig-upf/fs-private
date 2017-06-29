
#include <utils/static.hxx>
#include <problem_info.hxx>

namespace fs0 {


std::unique_ptr<StaticExtension>
StaticExtension::load_static_extension(const std::string& name, const ProblemInfo& info) {
	unsigned id = info.getSymbolId(name);
	const SymbolData& data = info.getSymbolData(id);
	unsigned arity = data.getArity();
	SymbolData::Type type = data.getType();
	assert(type == SymbolData::Type::PREDICATE || type == SymbolData::Type::FUNCTION);
	std::vector<type_id> sym_signature_types = info.get_type_ids(data.getSignature());
	//! We add the codomain here, if it is a function
	if (data.getType() == SymbolData::Type::FUNCTION )
		sym_signature_types.push_back(info.get_type_id(data.getCodomainType()));
	std::string filename = info.getDataDir() + "/" + name + ".data";
	StaticExtension* extension = nullptr;

	if (arity == 0) {
		extension = new ZeroaryFunction(Serializer::deserialize0AryElement(filename, sym_signature_types));

	} else if (arity == 1) {
		if (type == SymbolData::Type::PREDICATE) extension = new UnaryPredicate(Serializer::deserializeUnarySet(filename, sym_signature_types));
		else extension = new UnaryFunction(Serializer::deserializeUnaryMap(filename, sym_signature_types));

	} else if (arity == 2) {
		if (type == SymbolData::Type::PREDICATE) extension = new BinaryPredicate(Serializer::deserializeBinarySet(filename, sym_signature_types));
		else extension = new BinaryFunction(Serializer::deserializeBinaryMap(filename, sym_signature_types));

	} else if (arity == 3) {
		if (type == SymbolData::Type::PREDICATE) extension = new Arity3Predicate(Serializer::deserializeArity3Set(filename, sym_signature_types));
		else extension = new Arity3Function(Serializer::deserializeArity3Map(filename, sym_signature_types));

	} else if (arity == 4) {
		if (type == SymbolData::Type::PREDICATE) extension = new Arity4Predicate(Serializer::deserializeArity4Set(filename, sym_signature_types));
		else extension = new Arity4Function(Serializer::deserializeArity4Map(filename, sym_signature_types));


	} else WORK_IN_PROGRESS("Such high symbol arities have not yet been implemented");

	return std::unique_ptr<StaticExtension>(extension);
}

} // namespaces
