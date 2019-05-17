

#include <gtest/gtest.h>

#include <fstrips/language_info.hxx>

using namespace fs0;

class FStripsCore : public testing::Test {
	
};

//!
TEST_F(FStripsCore, ObjectId) {
	
	ASSERT_THROW(value<int>(object_id::TRUE), type_mismatch_error);
	ASSERT_NO_THROW(value<bool>(object_id::TRUE));

	
	ASSERT_TRUE((bool)object_id::TRUE);
	ASSERT_TRUE(value<bool>(object_id::TRUE));
	
	
	object_id int17a = make_obj<int32_t>(17);
	object_id int17b = make_obj<int32_t>(17);
	
	// Test equality, hashing, packing, unpacking.
	auto hasher = std::hash<object_id>();
	ASSERT_EQ(int17a, int17b);
	ASSERT_EQ(hasher(int17a), hasher(int17b));
	
	ASSERT_EQ(int(int17a), 17);
	ASSERT_EQ(value<int>(int17a), 17);
};


TEST_F(FStripsCore, LanguageInfo) {
// 	fs::StateVariable sv(0, nullptr);
// 	
// 	ASSERT_TRUE(fs::flat(sv));
// 	ASSERT_EQ(fs::nestedness(sv), 0);
// 	
// 	ASSERT_EQ(fs::all_terms(sv).size(), 1);
// 	ASSERT_EQ(fs::all_terms(sv)[0], &sv);
	
	
	fstrips::LanguageInfo lang;
	TypeIdx block_t = lang.add_fs_type("block");
	ASSERT_EQ(block_t, 1); // First type added
	ASSERT_EQ(lang.get_typename(block_t), "block");
	ASSERT_EQ(lang.get_type_id("block"), type_id::object_t);
	
	auto id = lang.add_object("b1", block_t);
};
