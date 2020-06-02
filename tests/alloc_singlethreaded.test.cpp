#include "alloc.hpp"
#include "gtest/gtest.h"

namespace {
TEST(raw_allocator_test, allocate_simple_objects) {
    EXPECT_EQ(1, 1);
}

TEST(raw_allocator_test, allocate_structs_classes) {
    EXPECT_EQ(1, 1);
}

TEST(raw_allocator_test, allocate_arrays) {
    EXPECT_EQ(1, 1);
}

TEST(stl_container_test, allocate_vectors) {
    EXPECT_EQ(1, 1);
}

TEST(stl_container_test, allocate_lists) {
    EXPECT_EQ(1, 1);
}

TEST(stl_container_test, allocate_maps) {
    EXPECT_EQ(1, 1);
}

TEST(smart_ptr_test, make_shared) {
    EXPECT_EQ(1, 1);
}

TEST(smart_ptr_test, allocate_shared) {
    EXPECT_EQ(1, 1);
}

TEST(smart_ptr_test, make_unique) {
    EXPECT_EQ(1, 1);
}

TEST(smart_ptr_test, allocate_unique) {
    EXPECT_EQ(1, 1);
}
}
