
#pragma once

#include <ostream>
#include <vector>
#include <set>
#include <map>

namespace aptk { namespace core {
	
class Serializer {
public:
	typedef std::map<int, int> UnaryMap;
	typedef std::map<std::pair<int, int>, int> BinaryMap;
	typedef std::set<int> UnarySet;
	typedef std::set<std::pair<int, int>> BinarySet;
	
	
	//! (de) serialize a unary map
	static std::ostream& serialize(std::ostream& os, const UnaryMap& map);
	static UnaryMap deserializeUnaryMap(std::istream& is);
	
	//! (de) serialize a binary map
	static std::ostream& serialize(std::ostream& os, const BinaryMap& map);
	static BinaryMap deserializeBinaryMap(std::istream& is);
	
	
	//! (de) serialize a unary map
	static std::ostream& serialize(std::ostream& os, const UnarySet& set);
	static UnarySet deserializeUnarySet(std::istream& is);
	
	//! (de) serialize a binary map
	static std::ostream& serialize(std::ostream& os, const BinarySet& set);
	static BinarySet deserializeBinarySet(std::istream& is);
	
protected:
	static std::vector<int> deserializeLine(const std::string& line);
	
};


} } // namespaces
