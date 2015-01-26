
#include <string>
#include <fstream>
#include <utils/serializer.hxx>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp>
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




	
void Serializer::deserialize(const std::string& filename, DataInserter& inserter) {
	std::ifstream is(filename);
	std::string line;
	while (std::getline(is, line)) {
		inserter(deserializeLine(line));
	}
}

int Serializer::deserialize0AryElement(const std::string& filename) {
	int data;
	DataInserter inserter = [&data](const std::vector<int>& elems) { assert(elems.size() == 1); data = elems[0]; };
	deserialize(filename, inserter);
	return data;
}

std::ostream& Serializer::serialize(std::ostream& os, const Serializer::UnaryMap& map) {
	for (const auto& it:map) {
		os << it.first << "," << it.second << std::endl;
	}
	return os;
}

Serializer::UnaryMap Serializer::deserializeUnaryMap(const std::string& filename) {
	UnaryMap data;
	DataInserter inserter = [&data](const std::vector<int>& elems) { assert(elems.size() == 2); data.insert(std::make_pair(elems[0], elems[1])); };
	deserialize(filename, inserter);
	return data;
}

Serializer::BinaryMap Serializer::deserializeBinaryMap(const std::string& filename) {
	BinaryMap data;
	DataInserter inserter = [&data](const std::vector<int>& elems) { assert(elems.size() == 3); data.insert(std::make_pair(std::make_pair(elems[0], elems[1]), elems[2]));};
	deserialize(filename, inserter);
	return data;
}

Serializer::UnarySet Serializer::deserializeUnarySet(const std::string& filename) {
	UnarySet data;
	DataInserter inserter = [&data](const std::vector<int>& elems) { assert(elems.size() == 1); data.insert(elems[0]);};
	deserialize(filename, inserter);
	return data;
}

Serializer::BinarySet Serializer::deserializeBinarySet(const std::string& filename) {
	BinarySet data;
	DataInserter inserter = [&data](const std::vector<int>& elems) { assert(elems.size() == 2); data.insert(std::make_pair(elems[0], elems[1])); };
	deserialize(filename, inserter);
	return data;
}

Serializer::Arity3Set Serializer::deserializeArity3Set(const std::string& filename) {
	Arity3Set data;
	DataInserter inserter = [&data](const std::vector<int>& elems) { assert(elems.size() == 3); data.insert(std::make_tuple(elems[0], elems[1], elems[2])); };
	deserialize(filename, inserter);
	return data;
}

Serializer::Arity4Set Serializer::deserializeArity4Set(const std::string& filename) {
	Arity4Set data;
	DataInserter inserter = [&data](const std::vector<int>& elems) { assert(elems.size() == 4); data.insert(std::make_tuple(elems[0], elems[1], elems[2], elems[3])); };
	deserialize(filename, inserter);
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

std::vector<int> Serializer::deserializeLine(const std::string& line) {
	std::vector<std::string> strings;
	boost::split(strings, line, boost::is_any_of(","));
	std::vector<int> result;
	result.reserve(strings.size());
	for (const std::string& str:strings) {
		result.push_back(boost::lexical_cast<int>(str));
	}
	return result;
}

} // namespaces
