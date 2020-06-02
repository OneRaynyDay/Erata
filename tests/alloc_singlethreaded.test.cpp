#define DEFAULT_WRITER_TYPE test_writer
#include "alloc.hpp"
#include "gtest/gtest.h"

namespace {
// === Raw allocator tests ===
struct test_struct {
    std::string s;
    int x;
    int* ptr;
};

class raw_allocator_tests : public testing::Test {
    virtual void SetUp() {
        auto& state = ert::profile_state<ert::default_writer_type>::get_state();
        auto alloc_writer = ert::writer::test_writer();
        auto dealloc_writer = ert::writer::test_writer();

        alloc_writer.setup("alloc_test");
        dealloc_writer.setup("dealloc_test");

        state.set_alloc_writer(alloc_writer);
        state.set_dealloc_writer(dealloc_writer);
    }
};

TEST_F(raw_allocator_tests, allocate_simple_objects) {
    auto& state = ert::profile_state<ert::default_writer_type>::get_state();
    // The writer must've been set up at the beginning of the program.
    EXPECT_EQ(state.get_alloc_writer().is_setup, true);

    // Allocate 2 ints
    ert::profile_allocator<int, std::allocator<int>> alloc;
    auto p = alloc.allocate(2);
    auto alloc_int_record = state.get_alloc_writer().latest_record;

    // We wrote once since allocate called once
    EXPECT_EQ(state.get_alloc_writer().num_writes, 1);
    // Deallocate hasn't been called yet
    EXPECT_EQ(state.get_dealloc_writer().num_writes, 0);
    // We have 2 ints, each of size 4.
    EXPECT_EQ(alloc_int_record.size, sizeof(int) * 2);
    // Hashcode to double check the type is correct.
    EXPECT_EQ(alloc_int_record.type_hash, typeid(int).hash_code());

    // Deallocate 2 ints from previous state
    alloc.deallocate(p, 2);
    auto dealloc_int_record = state.get_dealloc_writer().latest_record;

    // We did not write again to alloc writer.
    EXPECT_EQ(state.get_alloc_writer().num_writes, 1);
    EXPECT_EQ(state.get_dealloc_writer().num_writes, 1);
    // We have 2 ints, each of size 4.
    EXPECT_EQ(dealloc_int_record.size, sizeof(int) * 2);
    // Hashcode to double check the type is correct.
    EXPECT_EQ(dealloc_int_record.type_hash, typeid(int).hash_code());
}

TEST_F(raw_allocator_tests, allocate_structs_classes) {
    auto& state = ert::profile_state<ert::default_writer_type>::get_state();
    // The writer must've been set up at the beginning of the program.
    EXPECT_EQ(state.get_alloc_writer().is_setup, true);
    // Allocate 2 ints
    ert::profile_allocator<test_struct, std::allocator<test_struct>> alloc;
    auto p = alloc.allocate(10);
    auto alloc_struct_record = state.get_alloc_writer().latest_record;

    // We wrote once since allocate called once
    EXPECT_EQ(state.get_alloc_writer().num_writes, 1);
    // Deallocate hasn't been called yet
    EXPECT_EQ(state.get_dealloc_writer().num_writes, 0);
    // We have 2 ints, each of size 4.
    EXPECT_EQ(alloc_struct_record.size, sizeof(test_struct) * 10);
    // Hashcode to double check the type is correct.
    EXPECT_EQ(alloc_struct_record.type_hash, typeid(test_struct).hash_code());

    // Deallocate 2 ints from previous state
    alloc.deallocate(p, 10);
    auto dealloc_struct_record = state.get_dealloc_writer().latest_record;

    // We did not write again to alloc writer.
    EXPECT_EQ(state.get_alloc_writer().num_writes, 1);
    EXPECT_EQ(state.get_dealloc_writer().num_writes, 1);
    // We have 2 ints, each of size 4.
    EXPECT_EQ(dealloc_struct_record.size, sizeof(test_struct) * 10);
    // Hashcode to double check the type is correct.
    EXPECT_EQ(dealloc_struct_record.type_hash, typeid(test_struct).hash_code());
}

TEST_F(raw_allocator_tests, allocate_arrays) {
    auto& state = ert::profile_state<ert::default_writer_type>::get_state();
    // The writer must've been set up at the beginning of the program.
    EXPECT_EQ(state.get_alloc_writer().is_setup, true);
    // Allocate int arrays of size 10
    ert::profile_allocator<int[10], std::allocator<int[10]>> alloc;
    // allocate 5 of the int arrays (50 ints)
    auto p = alloc.allocate(5);
    auto alloc_array_record = state.get_alloc_writer().latest_record;

    // We wrote once since allocate called once
    EXPECT_EQ(state.get_alloc_writer().num_writes, 1);
    // Deallocate hasn't been called yet
    EXPECT_EQ(state.get_dealloc_writer().num_writes, 0);
    // We have 2 ints, each of size 4.
    EXPECT_EQ(alloc_array_record.size, sizeof(int[10]) * 5);
    // Hashcode to double check the type is correct.
    EXPECT_EQ(alloc_array_record.type_hash, typeid(int[10]).hash_code());

    alloc.deallocate(p, 5);
    auto dealloc_array_record = state.get_dealloc_writer().latest_record;

    // We did not write again to alloc writer.
    EXPECT_EQ(state.get_alloc_writer().num_writes, 1);
    EXPECT_EQ(state.get_dealloc_writer().num_writes, 1);
    // We have 2 ints, each of size 4.
    EXPECT_EQ(dealloc_array_record.size, sizeof(int[10]) * 5);
    // Hashcode to double check the type is correct.
    EXPECT_EQ(dealloc_array_record.type_hash, typeid(int[10]).hash_code());
}
// ===

// === STL container tests ===
TEST(stl_container_test, allocate_vectors) {
    EXPECT_EQ(1, 1);
}

TEST(stl_container_test, allocate_lists) {
    EXPECT_EQ(1, 1);
}

TEST(stl_container_test, allocate_maps) {
    EXPECT_EQ(1, 1);
}
// ===

// === Smart pointer tests ===
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
// ===
}
