
#include "static.hxx"

#include <fs/core/problem_info.hxx>
#include <lapkt/tools/logging.hxx>

#include <iostream>

namespace fs0 {

std::ostream& operator<<( std::ostream& os, const StaticExtension& ex )
{
	return ex.print(os, ProblemInfo::getInstance() );
}

std::unique_ptr<StaticExtension>
StaticExtension::load_static_extension(const std::string& name, const ProblemInfo& info) {
	unsigned id = info.getSymbolId(name);
	const SymbolData& data = info.getSymbolData(id);
	unsigned arity = data.getArity();
	SymbolData::Type type = data.getType();
	assert(type == SymbolData::Type::PREDICATE || type == SymbolData::Type::FUNCTION);
	std::vector<type_id> sym_signature_types = info.get_type_ids(data.getSignature());

	if (data.getType() == SymbolData::Type::FUNCTION) { // We add the codomain here, if it is a function
        sym_signature_types.push_back(info.get_type_id(data.getCodomainType()));
	}
	std::string filename = info.getDataDir() + "/" + name + ".data";
	StaticExtension* extension = nullptr;

	if (arity == 0) {
		if ( type == SymbolData::Type::PREDICATE ) {
			std::ifstream is(filename);
            // If the extension file does not exist, we assume the atom is a false nullary atom
            bool ext_file_exists = !is.fail();
            extension = new ZeroaryFunction(make_object(ext_file_exists));
		}
		else {
			extension = new ZeroaryFunction(Serializer::deserialize0AryElement(filename, sym_signature_types));
		}


	} else if (arity == 1) {
		if (type == SymbolData::Type::PREDICATE) extension = new UnaryPredicate(Serializer::deserializeUnarySet(filename, sym_signature_types));
		else extension = new UnaryFunction(Serializer::deserializeUnaryMap(filename, sym_signature_types));

	} else if (arity == 2) {
		if (type == SymbolData::Type::PREDICATE) extension = new BinaryPredicate(Serializer::deserializeBinarySet(filename, sym_signature_types));
		else extension = new BinaryFunction(Serializer::deserializeBinaryMap(filename, sym_signature_types));

	} else if (arity == 3) {
		if (type == SymbolData::Type::PREDICATE) extension = new Arity3Predicate(Serializer::deserializeArity3Set(filename, sym_signature_types));
		else extension = new Arity3Function(Serializer::deserializeArity3Map(filename, sym_signature_types));

	} else {
        if (type == SymbolData::Type::PREDICATE) extension = new ArbitraryArityPredicate(Serializer::deserializeSet(filename, sym_signature_types));
        else extension = new ArbitraryArityFunction(Serializer::deserializeMap(filename, sym_signature_types));
	}

	LPT_DEBUG("loader", "Loaded extension for symbol '" << name << "'\n\t data: ");
	extension->print(lapkt::tools::Logger::instance().log("DEBUG", "loader"), info) << std::endl;

	return std::unique_ptr<StaticExtension>(extension);
}

std::ostream&
ZeroaryFunction::print( std::ostream& os, const ProblemInfo& info ) const {
	os << "[";
	os << info.object_name(_data);
	os << "]";

	return os;
}

std::ostream&
UnaryFunction::print( std::ostream& os, const ProblemInfo& info ) const {
	os << "[";
	for (const auto& entry : _data ) {
		os << "( " << info.object_name(entry.first) << ", " << info.object_name(entry.second) << " ), ";
	}
	os << "]";

	return os;
}

std::ostream&
UnaryPredicate::print( std::ostream& os, const ProblemInfo& info ) const {
	os << "[";
	for (const auto& entry : _data ) {
		os << "( " << info.object_name(entry) << " ), ";
	}
	os << "]";
	return os;
}

std::ostream&
BinaryFunction::print( std::ostream& os, const ProblemInfo& info ) const {
	os << "[";
	for (const auto& entry : _data ) {
		os << "( ";
		os << info.object_name(entry.first.first) << ", ";
		os << info.object_name(entry.first.second) << ", ";
		os << info.object_name(entry.second) << " ), ";
	}
	os << "]";
	return os;
}

std::ostream&
BinaryPredicate::print( std::ostream& os, const ProblemInfo& info ) const {
	os << "[";
	for (const auto& entry : _data ) {
		os << "( ";
		os << info.object_name(entry.first) << ", ";
		os << info.object_name(entry.second);

		os << " ), ";
	}
	os << "]";
	return os;
}

std::ostream&
Arity3Function::print( std::ostream& os, const ProblemInfo& info ) const {
	os << "[";
	for (const auto& entry : _data ) {
		os << "( ";
		object_id x, y, z;
		std::tie(x,y,z) = entry.first;
		os << info.object_name(x) << ", ";
		os << info.object_name(y) << ", ";
		os << info.object_name(z) << ", ";

		os << info.object_name(entry.second) << " ), ";
	}
	os << "]";
	return os;
}

std::ostream&
Arity3Predicate::print( std::ostream& os, const ProblemInfo& info ) const {
	os << "[";
	for (const auto& entry : _data ) {
		os << "( ";
		object_id x, y, z;
		std::tie(x,y,z) = entry;
		os << info.object_name(x) << ", ";
		os << info.object_name(y) << ", ";
		os << info.object_name(z);

		os << " ), ";
	}
	os << "]";
	return os;
}

std::ostream&
ArbitraryArityFunction::print( std::ostream& os, const ProblemInfo& info ) const {
	os << "[";
	for (const auto& e:_data) {
		os << "( ";
		auto sz = e.first.size();
		for (unsigned i = 0; i < sz; ++i) {
            os << info.object_name(e.first[i]);
            if (i < sz-1) os << ", ";
		}
		os << info.object_name(e.second) << " ), ";
	}
	os << "]";
	return os;
}

std::ostream&
ArbitraryArityPredicate::print( std::ostream& os, const ProblemInfo& info ) const {
	os << "[";
	for (const auto& e:_data) {
		os << "( ";
        auto sz = e.size();
        for (unsigned i = 0; i < sz; ++i) {
            os << info.object_name(e[i]);
            if (i < sz-1) os << ", ";
        }
		os << " ), ";
	}
	os << "]";
	return os;
}

} // namespaces
