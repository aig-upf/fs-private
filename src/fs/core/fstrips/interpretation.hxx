
#pragma once

#include <fs/core/fstrips/fol.hxx>

#include <unordered_set>

namespace fs0 { namespace fstrips {

class RelationalExtension;

//!
class Interpretation {
public:
    Interpretation()
    {}

	~Interpretation() = default;
	Interpretation(const Interpretation&) = default;
	Interpretation(Interpretation&&) = default;
	Interpretation& operator=(const Interpretation&) = delete;
	Interpretation& operator=(Interpretation&&) = default;
	
	//! Prints a representation of the object to the given stream.
//	friend std::ostream& operator<<(std::ostream &os, const Interpretation& o);
//	virtual std::ostream& print(std::ostream& os, const LanguageInfo& info) const = 0;

	//! Accessors

protected:
    std::vector<RelationalExtension> predicates_;
};

class RelationalExtension {
public:
	using tuple_t = std::vector<unsigned>;
	RelationalExtension() {}

protected:
	//! A set with all tuples that make part of the extension
	std::unordered_set<tuple_t, boost::hash<tuple_t>> data_;
};


} } // namespaces
