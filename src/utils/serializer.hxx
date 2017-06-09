
#pragma once

#include <ostream>
#include <vector>
#include <set>
#include <map>
#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <functional>

#include <base.hxx>

namespace fs0 {
	
class Serializer {
public:
	typedef std::map<object_id, object_id> UnaryMap;
	typedef boost::container::flat_map<object_id, object_id> BoostUnaryMap;
	
	typedef std::map<std::pair<object_id, object_id>, object_id> BinaryMap;
	typedef boost::container::flat_map<std::pair<object_id, object_id>, object_id> BoostBinaryMap;
	
	typedef std::map<std::tuple<object_id, object_id, object_id>, object_id> Arity3Map;
	typedef boost::container::flat_map<std::tuple<object_id, object_id, object_id>, object_id> BoostArity3Map;
	
	typedef std::map<std::tuple<object_id, object_id, object_id, object_id>, object_id> Arity4Map;
	typedef boost::container::flat_map<std::tuple<object_id, object_id, object_id, object_id>, object_id> BoostArity4Map;
	
	typedef std::set<object_id> UnarySet;
	typedef boost::container::flat_set<object_id> BoostUnarySet;
	
	typedef std::set<std::pair<object_id, object_id>> BinarySet;
	typedef boost::container::flat_set<std::pair<object_id, object_id>> BoostBinarySet;
	
	typedef std::set<std::tuple<object_id, object_id, object_id>> Arity3Set;
	typedef boost::container::flat_set<std::tuple<object_id, object_id, object_id>> BoostArity3Set;
	
	typedef std::set<std::tuple<object_id, object_id, object_id, object_id>> Arity4Set;
	typedef boost::container::flat_set<std::tuple<object_id, object_id, object_id, object_id>> BoostArity4Set;
	
	
	//! For the sake of completeness
	static object_id deserialize0AryElement(const std::string& filename);
	
	
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
	
	
	typedef std::function<void (const std::vector<object_id>&)> DataInserter;
	static void deserialize(const std::string& filename, DataInserter& inserter);
	
	//! Boost (de)serialization
	template <typename T>
	static void BoostDeserialize(const std::string& filename, T& data);
	
	template <typename T>
	static void BoostSerialize(const std::string& filename, const T& data);
	
	static BinaryMap bDeserializeBinaryMap(const std::string& filename);
	
	static std::vector<object_id> deserializeLine(const std::string& line, const std::string& separators = ",");
	
	template <typename T>
	static std::vector<T> deserialize_line(const std::string& line, const std::string& separators = ",");
};


} // namespaces
