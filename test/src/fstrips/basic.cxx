

#include <gtest/gtest.h>

#include <languages/fstrips/language.hxx>
#include <languages/fstrips/operations.hxx>

using namespace fs0;
namespace fs = fs0::language::fstrips;

class TermsBasic : public testing::Test {
	
};

//!
TEST_F(TermsBasic, BasicProperties) {
	fs::StateVariable sv(0, nullptr);
	
	ASSERT_TRUE(fs::flat(sv));
	ASSERT_EQ(fs::nestedness(sv), 0);
	
	ASSERT_EQ(fs::all_terms(sv).size(), 1);
	ASSERT_EQ(fs::all_terms(sv)[0], &sv);

	
	
	
	
};
