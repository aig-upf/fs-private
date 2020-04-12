
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
//	using UnaryMap = std::map<object_id, object_id>;
    using UnaryMap = boost::container::flat_map<object_id, object_id>;

//    using BinaryMap = std::map<std::pair<object_id, object_id>, object_id>;
    using BinaryMap = boost::container::flat_map<std::pair<object_id, object_id>, object_id> ;

//    using Arity3Map = std::map<std::tuple<object_id, object_id, object_id>, object_id>;
    using Arity3Map = boost::container::flat_map<std::tuple<object_id, object_id, object_id>, object_id>;

//    using UnarySet = std::set<object_id>;
    using UnarySet = boost::container::flat_set<object_id>;

//    using BinarySet = std::set<std::pair<object_id, object_id>>;
    using BinarySet = boost::container::flat_set<std::pair<object_id, object_id>>;

//    using Arity3Set = std::set<std::tuple<object_id, object_id, object_id>>;
    using Arity3Set = boost::container::flat_set<std::tuple<object_id, object_id, object_id>>;


	using predicate_t = boost::container::flat_set<std::vector<object_id>>;
	using function_t = boost::container::flat_map<std::vector<object_id>, object_id>;


	//! For the sake of completeness
	static object_id deserialize0AryElement(const std::string& filename, const std::vector<type_id>& sym_signature_types);


	//! Map (de)serialization
	static std::ostream& serialize(std::ostream& os, const UnaryMap& map);
	static std::ostream& serialize(std::ostream& os, const BinaryMap& map);
	static UnaryMap deserializeUnaryMap(const std::string& filename, const std::vector<type_id>& sym_signature_types );
	static BinaryMap deserializeBinaryMap(const std::string& filename, const std::vector<type_id>& sym_signature_types);
	static Arity3Map deserializeArity3Map(const std::string& filename, const std::vector<type_id>& sym_signature_types);
	static function_t deserializeMap(const std::string& filename, const std::vector<type_id>& sym_signature_types);


	//! Set (de)serialization
	static std::ostream& serialize(std::ostream& os, const UnarySet& set);
	static std::ostream& serialize(std::ostream& os, const BinarySet& set);
	static UnarySet deserializeUnarySet(const std::string& filename, const std::vector<type_id>& sym_signature_types);
	static BinarySet deserializeBinarySet(const std::string& filename, const std::vector<type_id>& sym_signature_types);
	static Arity3Set deserializeArity3Set(const std::string& filename, const std::vector<type_id>& sym_signature_types);
	static predicate_t deserializeSet(const std::string& filename, const std::vector<type_id>& sym_signature_types);


	using DataInserter = std::function<void (std::vector<object_id>&&)>;
	static void deserialize(const std::string& filename, DataInserter& inserter, const std::vector<type_id>& sym_signature_types);

	//! Boost (de)serialization
//	template <typename T>
//	static void BoostDeserialize(const std::string& filename, T& data);

//	template <typename T>
//	static void BoostSerialize(const std::string& filename, const T& data);

//	static BinaryMap bDeserializeBinaryMap(const std::string& filename);

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
