
#pragma once

#include <ostream>
#include <vector>
#include <set>
#include <map>
#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <functional>

namespace fs0 {
	
class Serializer {
public:
	typedef std::map<int, int> UnaryMap;
	typedef boost::container::flat_map<int, int> BoostUnaryMap;
	
	typedef std::map<std::pair<int, int>, int> BinaryMap;
	typedef boost::container::flat_map<std::pair<int, int>, int> BoostBinaryMap;
	
	typedef std::map<std::tuple<int, int, int>, int> Arity3Map;
	typedef boost::container::flat_map<std::tuple<int, int, int>, int> BoostArity3Map;
	
	typedef std::map<std::tuple<int, int, int, int>, int> Arity4Map;
	typedef boost::container::flat_map<std::tuple<int, int, int, int>, int> BoostArity4Map;
	
	typedef std::set<int> UnarySet;
	typedef boost::container::flat_set<int> BoostUnarySet;
	
	typedef std::set<std::pair<int, int>> BinarySet;
	typedef boost::container::flat_set<std::pair<int, int>> BoostBinarySet;
	
	typedef std::set<std::tuple<int, int, int>> Arity3Set;
	typedef boost::container::flat_set<std::tuple<int, int, int>> BoostArity3Set;
	
	typedef std::set<std::tuple<int, int, int, int>> Arity4Set;
	typedef boost::container::flat_set<std::tuple<int, int, int, int>> BoostArity4Set;
	
	
	//! For the sake of completeness
	static int deserialize0AryElement(const std::string& filename);
	
	
	//! Map (de)serialization
	static std::ostream& serialize(std::ostream& os, const UnaryMap& map);
	static std::ostream& serialize(std::ostream& os, const BinaryMap& map);
	static BoostUnaryMap deserializeUnaryMap(const std::string& filename);
	static BoostBinaryMap deserializeBinaryMap(const std::string& filename);
	static BoostArity3Map deserializeArity3Map(const std::string& filename);
	static BoostArity4Map deserializeArity4Map(const std::string& filename);
	
	
	//! Set (de)serialization
	static std::ostream& serialize(std::ostream& os, const UnarySet& set);
	static std::ostream& serialize(std::ostream& os, const BinarySet& set);
	static BoostUnarySet deserializeUnarySet(const std::string& filename);
	static BoostBinarySet deserializeBinarySet(const std::string& filename);
	static BoostArity3Set deserializeArity3Set(const std::string& filename);
	static BoostArity4Set deserializeArity4Set(const std::string& filename);
	
	
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


} // namespaces
