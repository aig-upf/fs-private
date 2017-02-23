
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
	
	std::string filename = info.getDataDir() + "/" + name + ".data";
	StaticExtension* extension = nullptr;
	
	if (arity == 0) {
		extension = new ZeroaryFunction(Serializer::deserialize0AryElement(filename));
		
	} else if (arity == 1) {
		if (type == SymbolData::Type::PREDICATE) extension = new UnaryPredicate(Serializer::deserializeUnarySet(filename));
		else extension = new UnaryFunction(Serializer::deserializeUnaryMap(filename));
		
	} else if (arity == 2) {
		if (type == SymbolData::Type::PREDICATE) extension = new BinaryPredicate(Serializer::deserializeBinarySet(filename));
		else extension = new BinaryFunction(Serializer::deserializeBinaryMap(filename));
		
	} else if (arity == 3) {
		if (type == SymbolData::Type::PREDICATE) extension = new Arity3Predicate(Serializer::deserializeArity3Set(filename));
		else extension = new Arity3Function(Serializer::deserializeArity3Map(filename));
		
	} else if (arity == 4) {
		if (type == SymbolData::Type::PREDICATE) extension = new Arity4Predicate(Serializer::deserializeArity4Set(filename));
		else extension = new Arity4Function(Serializer::deserializeArity4Map(filename));


	} else WORK_IN_PROGRESS("Such high symbol arities have not yet been implemented");
	
	return std::unique_ptr<StaticExtension>(extension);
}

} // namespaces
