
#pragma once

#include <ostream>
#include <vector>
#include <set>
#include <map>
#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <functional>

#include <fs/core/base.hxx>

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
	static object_id deserialize0AryElement(const std::string& filename, const std::vector<type_id>& sym_signature_types);


	//! Map (de)serialization
	static std::ostream& serialize(std::ostream& os, const UnaryMap& map);
	static std::ostream& serialize(std::ostream& os, const BinaryMap& map);
	static BoostUnaryMap deserializeUnaryMap(const std::string& filename, const std::vector<type_id>& sym_signature_types );
	static BoostBinaryMap deserializeBinaryMap(const std::string& filename, const std::vector<type_id>& sym_signature_types);
	static BoostArity3Map deserializeArity3Map(const std::string& filename, const std::vector<type_id>& sym_signature_types);
	static BoostArity4Map deserializeArity4Map(const std::string& filename, const std::vector<type_id>& sym_signature_types);


	//! Set (de)serialization
	static std::ostream& serialize(std::ostream& os, const UnarySet& set);
	static std::ostream& serialize(std::ostream& os, const BinarySet& set);
	static BoostUnarySet deserializeUnarySet(const std::string& filename, const std::vector<type_id>& sym_signature_types);
	static BoostBinarySet deserializeBinarySet(const std::string& filename, const std::vector<type_id>& sym_signature_types);
	static BoostArity3Set deserializeArity3Set(const std::string& filename, const std::vector<type_id>& sym_signature_types);
	static BoostArity4Set deserializeArity4Set(const std::string& filename, const std::vector<type_id>& sym_signature_types);


	typedef std::function<void (const std::vector<object_id>&)> DataInserter;
	static void deserialize(const std::string& filename, DataInserter& inserter, const std::vector<type_id>& sym_signature_types);

	//! Boost (de)serialization
	template <typename T>
	static void BoostDeserialize(const std::string& filename, T& data);

	template <typename T>
	static void BoostSerialize(const std::string& filename, const T& data);

	static BinaryMap bDeserializeBinaryMap(const std::string& filename);

	//! Deserialize a line of (typed) objects through a given object indexer, e.g. "a b c d" gets mapped into the
	//! object IDs corresponding to objects a, b, c, and d
	static std::vector<object_id>
	deserialize_line(const std::string& line,
	        const std::vector<type_id>& sym_signature_types,
	        const std::string& separators,
	        const std::function<unsigned(const std::string&)>& object_indexer);

    static std::vector<object_id>
    deserialize_line(const std::string& line,
                     const std::vector<type_id>& sym_signature_types,
                     const std::string& separators);
};


} // namespaces
