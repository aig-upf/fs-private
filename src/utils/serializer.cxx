
#include <string>
#include <fstream>
#include <utils/serializer.hxx>
#include <utils/serialize_tuple.hxx>
#include <boost/algorithm/string.hpp>
#include <utils/lexical_cast.hxx>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>


namespace fs0 {


template <typename T>
void Serializer::BoostDeserialize(const std::string& filename, T& data) {
	std::ifstream ifs(filename);
	if(ifs.is_open()){
		boost::archive::text_iarchive iarch(ifs);
		iarch >> data;
	}
	ifs.close();
}

//! Template instantiations
// template void Serializer::BoostDeserialize<Serializer::BinarySet>(const std::string& filename, Serializer::BinarySet& data);

template <typename T>
void Serializer::BoostSerialize(const std::string& filename, const T& data) {
	std::ofstream ofs(filename);
	boost::archive::text_oarchive oarch(ofs);
	oarch << data;
	ofs.close();
}

Serializer::BinaryMap Serializer::bDeserializeBinaryMap(const std::string& filename) {
	BinaryMap data;
	BoostDeserialize(filename, data);
	return data;
}

void Serializer::deserialize(const std::string& filename, DataInserter& inserter, const std::vector<type_id>& sym_signature_types) {
	std::ifstream is(filename);
	std::string line;
	while (std::getline(is, line)) {
		inserter(deserialize_line(line, sym_signature_types));
	}
}

object_id Serializer::deserialize0AryElement(const std::string& filename, const std::vector<type_id>& sym_signature_types) {
	object_id data;
	DataInserter inserter = [&data](const std::vector<object_id>& elems) { assert(elems.size() == 1); data = elems[0]; };
	deserialize(filename, inserter, sym_signature_types);
	return data;
}

std::ostream& Serializer::serialize(std::ostream& os, const Serializer::UnaryMap& map) {
	for (const auto& it:map) {
		os << it.first << "," << it.second << std::endl;
	}
	return os;
}

Serializer::BoostUnaryMap Serializer::deserializeUnaryMap(const std::string& filename, const std::vector<type_id>& sym_signature_types) {
	BoostUnaryMap data;
	DataInserter inserter = [&data](const std::vector<object_id>& elems) { assert(elems.size() == 2); data.insert(std::make_pair(elems[0], elems[1])); };
	deserialize(filename, inserter, sym_signature_types);
	return data;
}

Serializer::BoostBinaryMap Serializer::deserializeBinaryMap(const std::string& filename, const std::vector<type_id>& sym_signature_types) {
	BoostBinaryMap data;
	DataInserter inserter = [&data](const std::vector<object_id>& elems) { assert(elems.size() == 3); data.insert(std::make_pair(std::make_pair(elems[0], elems[1]), elems[2]));};
	deserialize(filename, inserter, sym_signature_types);
	return data;
}

Serializer::BoostArity3Map Serializer::deserializeArity3Map(const std::string& filename, const std::vector<type_id>& sym_signature_types) {
	BoostArity3Map data;
	DataInserter inserter = [&data](const std::vector<object_id>& elems) { assert(elems.size() == 4); data.insert(std::make_pair(std::make_tuple(elems[0], elems[1], elems[2]), elems[3]));};
	deserialize(filename, inserter, sym_signature_types);
	return data;
}
Serializer::BoostArity4Map Serializer::deserializeArity4Map(const std::string& filename, const std::vector<type_id>& sym_signature_types) {
	BoostArity4Map data;
	DataInserter inserter = [&data](const std::vector<object_id>& elems) { assert(elems.size() == 5); data.insert(std::make_pair(std::make_tuple(elems[0], elems[1], elems[2], elems[3]), elems[4]));};
	deserialize(filename, inserter, sym_signature_types);
	return data;
}

Serializer::BoostUnarySet Serializer::deserializeUnarySet(const std::string& filename, const std::vector<type_id>& sym_signature_types) {
	BoostUnarySet data;
	DataInserter inserter = [&data](const std::vector<object_id>& elems) { assert(elems.size() == 1); data.insert(elems[0]);};
	deserialize(filename, inserter, sym_signature_types);
	return data;
}

Serializer::BoostBinarySet Serializer::deserializeBinarySet(const std::string& filename, const std::vector<type_id>& sym_signature_types) {
	BoostBinarySet data;
	DataInserter inserter = [&data](const std::vector<object_id>& elems) { assert(elems.size() == 2); data.insert(std::make_pair(elems[0], elems[1])); };
	deserialize(filename, inserter, sym_signature_types);
	return data;
}

Serializer::BoostArity3Set Serializer::deserializeArity3Set(const std::string& filename, const std::vector<type_id>& sym_signature_types) {
	BoostArity3Set data;
	DataInserter inserter = [&data](const std::vector<object_id>& elems) { assert(elems.size() == 3); data.insert(std::make_tuple(elems[0], elems[1], elems[2])); };
	deserialize(filename, inserter, sym_signature_types);
	return data;
}

Serializer::BoostArity4Set Serializer::deserializeArity4Set(const std::string& filename, const std::vector<type_id>& sym_signature_types) {
	BoostArity4Set data;
	DataInserter inserter = [&data](const std::vector<object_id>& elems) { assert(elems.size() == 4); data.insert(std::make_tuple(elems[0], elems[1], elems[2], elems[3])); };
	deserialize(filename, inserter, sym_signature_types);
	return data;
}

std::ostream& Serializer::serialize(std::ostream& os, const Serializer::BinaryMap& map) {
	for (const auto& it:map) {
		os << it.first.first << "," << it.first.second << "," << it.second << std::endl;
	}
	return os;
}

std::ostream& Serializer::serialize(std::ostream& os, const Serializer::UnarySet& set) {
	for (const auto& it:set) {
		os << it << std::endl;
	}
	return os;
}

std::ostream& Serializer::serialize(std::ostream& os, const Serializer::BinarySet& set) {
	for (const auto& it:set) {
		os << it.first << "," << it.second << std::endl;
	}
	return os;
}

std::vector<object_id> Serializer::
deserialize_line(const std::string& line, const std::vector<type_id>& sym_signature_types, const std::string& separators) {
	if (line.size() == 0) return {};
	std::vector<std::string> strings;
	boost::split(strings, line, boost::is_any_of(separators));
	assert(strings.size() == sym_signature_types.size() );
	std::vector<object_id> result;
	result.reserve(strings.size());

	for ( unsigned i = 0; i < strings.size(); i++ ) {
		if ( sym_signature_types[i] == type_id::int_t ) {
			result.push_back( make_object(type_id::int_t, boost::lexical_cast<int>(strings[i])));
		}
		else if ( sym_signature_types[i] == type_id::float_t ) {
			result.push_back( make_object(type_id::int_t, boost::lexical_cast<float>(strings[i])));
		}
		else if ( sym_signature_types[i] == type_id::object_t ) {
			result.push_back( make_object(type_id::int_t, boost::lexical_cast<int>(strings[i])));
		}
		else {
			throw std::runtime_error("Could not interpret types of constants in line \n" + line );
		}
	}

	return result;
}

} // namespaces
