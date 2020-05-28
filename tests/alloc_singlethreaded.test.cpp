#include "alloc.hpp"
#include "gtest/gtest.h"

/// Honestly, testing this thing is going to be a complete shit-show
/// because we don't want virtualized methods(cause they're slow) and we don't want to
/// templatize the logger type cause then we'd get 1 singleton per template type, which is stupid
/// not to mention make the code itself unreadable. What we'll aim to do is run a few access patterns and
/// assert that the data is accurate in the specified log file.
namespace {
TEST(raw_allocator_test, allocate_simple_objects) {

}

TEST(raw_allocator_test, allocate_structs_classes) {

}

TEST(raw_allocator_test, allocate_arrays) {

}

TEST(stl_container_test, allocate_vectors) {

}

TEST(stl_container_test, allocate_lists) {

}

TEST(stl_container_test, allocate_)
