
#include <string>
#include <utils/serializer.hxx>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace aptk { namespace core {

std::ostream& Serializer::serialize(std::ostream& os, const Serializer::UnaryMap& map) {
	for (const auto& it:map) {
		os << it.first << "," << it.second << std::endl;
	}
	return os;
}

Serializer::UnaryMap Serializer::deserializeUnaryMap(std::istream& is) {
	UnaryMap map;
	std::string line;
	while (std::getline(is, line)) {
		std::vector<int> elems = deserializeLine(line);
		assert(elems.size() == 2);
		map.insert({elems[0], elems[1]});
	}
	return map;
}

std::ostream& Serializer::serialize(std::ostream& os, const Serializer::BinaryMap& map) {
	for (const auto& it:map) {
		os << it.first.first << "," << it.first.second << "," << it.second << std::endl;
	}
	return os;
}

Serializer::BinaryMap Serializer::deserializeBinaryMap(std::istream& is) {
	BinaryMap map;
	std::string line;
	while (std::getline(is, line)) {
		std::vector<int> elems = deserializeLine(line);
		assert(elems.size() == 3);
		map.insert({{elems[0], elems[1]}, elems[2]});
	}
	return map;
}


std::ostream& Serializer::serialize(std::ostream& os, const Serializer::UnarySet& set) {
	for (const auto& it:set) {
		os << it << std::endl;
	}
	return os;
}

Serializer::UnarySet Serializer::deserializeUnarySet(std::istream& is) {
	UnarySet set;
	std::string line;
	while (std::getline(is, line)) {
		std::vector<int> elems = deserializeLine(line);
		assert(elems.size() == 1);
		set.insert(elems[0]);
	}
	return set;
}


std::ostream& Serializer::serialize(std::ostream& os, const Serializer::BinarySet& set) {
	for (const auto& it:set) {
		os << it.first << "," << it.second << std::endl;
	}
	return os;
}

Serializer::BinarySet Serializer::deserializeBinarySet(std::istream& is) {
	BinarySet set;
	std::string line;
	while (std::getline(is, line)) {
		std::vector<int> elems = deserializeLine(line);
		assert(elems.size() == 2);
		set.insert({elems[0], elems[1]});
	}
	return set;
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

} } // namespaces
