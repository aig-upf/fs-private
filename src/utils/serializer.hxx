
#pragma once

#include <ostream>
#include <vector>
#include <set>
#include <map>
#include <functional>

namespace aptk { namespace core {
	
class Serializer {
public:
	typedef std::map<int, int> UnaryMap;
	typedef std::map<std::pair<int, int>, int> BinaryMap;
	typedef std::set<int> UnarySet;
	typedef std::set<std::pair<int, int>> BinarySet;
	
	
	//! Map (de)serialization
	static std::ostream& serialize(std::ostream& os, const UnaryMap& map);
	static std::ostream& serialize(std::ostream& os, const BinaryMap& map);
	static UnaryMap deserializeUnaryMap(const std::string& filename);
	static BinaryMap deserializeBinaryMap(const std::string& filename);
	
	
	//! Set (de)serialization
	static std::ostream& serialize(std::ostream& os, const UnarySet& set);
	static std::ostream& serialize(std::ostream& os, const BinarySet& set);
	static UnarySet deserializeUnarySet(const std::string& filename);
	static BinarySet deserializeBinarySet(const std::string& filename);
	
	typedef std::function<void (const std::vector<int>&)> DataInserter;
	static void deserialize(const std::string& filename, DataInserter& inserter);
	
	//! Boost (de)serialization
	template <typename T>
	static void BoostDeserialize(const std::string& filename, T& data);
	
	template <typename T>
	static void BoostSerialize(const std::string& filename, const T& data);
	
	static BinaryMap bDeserializeBinaryMap(const std::string& filename);
	
protected:
	static std::vector<int> deserializeLine(const std::string& line);
	
};


} } // namespaces
