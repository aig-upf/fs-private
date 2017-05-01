
#pragma once

#include <ostream>
#include <vector>
#include <set>
#include <map>
#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <functional>
#include <fs_types.hxx>

namespace fs0 {

class Serializer {
public:
	typedef std::map<ObjectIdx, ObjectIdx> UnaryMap;
	typedef boost::container::flat_map<ObjectIdx, ObjectIdx> BoostUnaryMap;

	typedef std::map<std::pair<ObjectIdx, ObjectIdx>, ObjectIdx> BinaryMap;
	typedef boost::container::flat_map<std::pair<ObjectIdx, ObjectIdx>, ObjectIdx> BoostBinaryMap;

	typedef std::map<std::tuple<ObjectIdx, ObjectIdx, ObjectIdx>, ObjectIdx> Arity3Map;
	typedef boost::container::flat_map<std::tuple<ObjectIdx, ObjectIdx, ObjectIdx>, ObjectIdx> BoostArity3Map;

	typedef std::map<std::tuple<ObjectIdx, ObjectIdx, ObjectIdx, ObjectIdx>, ObjectIdx> Arity4Map;
	typedef boost::container::flat_map<std::tuple<ObjectIdx, ObjectIdx, ObjectIdx, ObjectIdx>, ObjectIdx> BoostArity4Map;

	typedef std::set<ObjectIdx> UnarySet;
	typedef boost::container::flat_set<ObjectIdx> BoostUnarySet;

	typedef std::set<std::pair<ObjectIdx, ObjectIdx>> BinarySet;
	typedef boost::container::flat_set<std::pair<ObjectIdx, ObjectIdx>> BoostBinarySet;

	typedef std::set<std::tuple<ObjectIdx, ObjectIdx, ObjectIdx>> Arity3Set;
	typedef boost::container::flat_set<std::tuple<ObjectIdx, ObjectIdx, ObjectIdx>> BoostArity3Set;

	typedef std::set<std::tuple<ObjectIdx, ObjectIdx, ObjectIdx, ObjectIdx>> Arity4Set;
	typedef boost::container::flat_set<std::tuple<ObjectIdx, ObjectIdx, ObjectIdx, ObjectIdx>> BoostArity4Set;


	//! For the sake of completeness
	static ObjectIdx deserialize0AryElement(const std::string& filename);


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


	typedef std::function<void (const std::vector<ObjectIdx>&)> DataInserter;
	static void deserialize(const std::string& filename, DataInserter& inserter);

	//! Boost (de)serialization
	template <typename T>
	static void BoostDeserialize(const std::string& filename, T& data);

	template <typename T>
	static void BoostSerialize(const std::string& filename, const T& data);

	static BinaryMap bDeserializeBinaryMap(const std::string& filename);

	static std::vector<ObjectIdx> deserializeLine(const std::string& line, const std::string& separators = ",");
};


} // namespaces
