
#pragma once

#include <memory>
#include <vector>

#include "fixtures/base_fixture.hxx"
#include <constraints/alldiff.hxx>
#include <core_types.hxx>

using namespace aptk::core;

namespace aptk { namespace test { namespace constraints {

class ConstraintFixture : public testing::Test {
protected:
	//! Little helper to write more concise tests.
	DomainSet buildDomainSet(std::map<unsigned, std::vector<ObjectIdx>> domains) {
		DomainSet set;
		for (const auto& d:domains) {
			set.insert({d.first, DomainSetVector(d.second.begin(), d.second.end())});
		}
		return set;
	}
};

} } } // namespaces
