#define DEFAULT_WRITER_TYPE test_writer
#include "alloc.hpp"
#include "stl_containers.hpp"
#include "pointer_types.hpp"
#include "gtest/gtest.h"

// === Raw allocator tests ===
struct test_struct {
    std::string s;
    int x;
    int* ptr;
};

template <typename T>
class test_allocator : public std::allocator<T> {
public:
    test_allocator() = default;

    template <typename U>
    test_allocator(const test_allocator<U>& other) : std::allocator<T>(other){}

    template<class U>
    struct rebind {
        typedef test_allocator<U> other;
    };
};

class test_base : public testing::Test {
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

class raw_allocator_tests : public test_base {
};

class stl_container_tests : public test_base {
};

class smart_pointer_tests : public test_base {
};

TEST(allocator_construction_tests, create_allocator) {
    // Default construction
    ert::profile_allocator<int, std::allocator<int>> alloc_1;

    // Copy construction
    ert::profile_allocator<int, std::allocator<int>> alloc_2(alloc_1);

    // Copy construction from a different type
    ert::profile_allocator<double, std::allocator<double>> alloc_3(alloc_1);

    // Forwarding arguments (calling copy ctor of base)
    ert::profile_allocator<double> alloc_4(std::allocator<double>());
    ert::profile_allocator<double, test_allocator<double>> alloc_5(test_allocator<double>());
}

TEST_F(raw_allocator_tests, allocate_simple_objects) {
    auto& state = ert::profile_state<ert::default_writer_type>::get_state();
    // The writer must've been set up at the beginning of the program.
    ASSERT_EQ(state.get_alloc_writer().is_setup, true);

    // Allocate 2 ints
    ert::profile_allocator<int, std::allocator<int>> alloc;
    auto p = alloc.allocate(2);
    auto alloc_int_record = state.get_alloc_writer().latest_record;

    // We wrote once since allocate called once
    ASSERT_EQ(state.get_alloc_writer().num_writes, 1);
    // Deallocate hasn't been called yet
    ASSERT_EQ(state.get_dealloc_writer().num_writes, 0);
    // We have 2 ints, each of size 4.
    ASSERT_EQ(alloc_int_record.size, sizeof(int) * 2);
    // Hashcode to double check the type is correct.
    ASSERT_EQ(alloc_int_record.type_hash, typeid(int).hash_code());

    // Deallocate 2 ints from previous state
    alloc.deallocate(p, 2);
    auto dealloc_int_record = state.get_dealloc_writer().latest_record;

    // We did not write again to alloc writer.
    ASSERT_EQ(state.get_alloc_writer().num_writes, 1);
    ASSERT_EQ(state.get_dealloc_writer().num_writes, 1);
    // We have 2 ints, each of size 4.
    ASSERT_EQ(dealloc_int_record.size, sizeof(int) * 2);
    // Hashcode to double check the type is correct.
    ASSERT_EQ(dealloc_int_record.type_hash, typeid(int).hash_code());
}

TEST_F(raw_allocator_tests, allocate_structs_classes) {
    auto& state = ert::profile_state<ert::default_writer_type>::get_state();
    // The writer must've been set up at the beginning of the program.
    ASSERT_EQ(state.get_alloc_writer().is_setup, true);
    // Allocate 2 ints
    ert::profile_allocator<test_struct, std::allocator<test_struct>> alloc;
    auto p = alloc.allocate(10);
    auto alloc_struct_record = state.get_alloc_writer().latest_record;

    // We wrote once since allocate called once
    ASSERT_EQ(state.get_alloc_writer().num_writes, 1);
    // Deallocate hasn't been called yet
    ASSERT_EQ(state.get_dealloc_writer().num_writes, 0);
    // We have 2 ints, each of size 4.
    ASSERT_EQ(alloc_struct_record.size, sizeof(test_struct) * 10);
    // Hashcode to double check the type is correct.
    ASSERT_EQ(alloc_struct_record.type_hash, typeid(test_struct).hash_code());

    // Deallocate 2 ints from previous state
    alloc.deallocate(p, 10);
    auto dealloc_struct_record = state.get_dealloc_writer().latest_record;

    // We did not write again to alloc writer.
    ASSERT_EQ(state.get_alloc_writer().num_writes, 1);
    ASSERT_EQ(state.get_dealloc_writer().num_writes, 1);
    // We have 2 ints, each of size 4.
    ASSERT_EQ(dealloc_struct_record.size, sizeof(test_struct) * 10);
    // Hashcode to double check the type is correct.
    ASSERT_EQ(dealloc_struct_record.type_hash, typeid(test_struct).hash_code());
}

TEST_F(raw_allocator_tests, allocate_arrays) {
    auto& state = ert::profile_state<ert::default_writer_type>::get_state();
    // The writer must've been set up at the beginning of the program.
    ASSERT_EQ(state.get_alloc_writer().is_setup, true);
    // Allocate int arrays of size 10
    ert::profile_allocator<int[10], std::allocator<int[10]>> alloc;
    // allocate 5 of the int arrays (50 ints)
    auto p = alloc.allocate(5);
    auto alloc_array_record = state.get_alloc_writer().latest_record;

    // We wrote once since allocate called once
    ASSERT_EQ(state.get_alloc_writer().num_writes, 1);
    // Deallocate hasn't been called yet
    ASSERT_EQ(state.get_dealloc_writer().num_writes, 0);
    // We have 2 ints, each of size 4.
    ASSERT_EQ(alloc_array_record.size, sizeof(int[10]) * 5);
    // Hashcode to double check the type is correct.
    ASSERT_EQ(alloc_array_record.type_hash, typeid(int[10]).hash_code());

    alloc.deallocate(p, 5);
    auto dealloc_array_record = state.get_dealloc_writer().latest_record;

    // We did not write again to alloc writer.
    ASSERT_EQ(state.get_alloc_writer().num_writes, 1);
    ASSERT_EQ(state.get_dealloc_writer().num_writes, 1);
    // We have 2 ints, each of size 4.
    ASSERT_EQ(dealloc_array_record.size, sizeof(int[10]) * 5);
    // Hashcode to double check the type is correct.
    ASSERT_EQ(dealloc_array_record.type_hash, typeid(int[10]).hash_code());
}
// ===

// === STL container tests ===
TEST_F(stl_container_tests, allocate_vectors) {
    auto& state = ert::profile_state<ert::default_writer_type>::get_state();

    auto increase_and_check = [&]<typename VecType>(VecType& v) {
        v.push_back(1);
        auto alloc_array_record = state.get_alloc_writer().latest_record;
        // Because we don't know the growth ratio of vectors, we can't make
        // simple assumptions to use ASSERT_EQ even though 2^n seems to be
        // the common implementation right now.
        ASSERT_GE(alloc_array_record.size, v.size() * sizeof(int));
        ASSERT_GE(v.capacity() * sizeof(int), alloc_array_record.size);
    };

    // The writer must've been set up at the beginning of the program.
    ASSERT_EQ(state.get_alloc_writer().is_setup, true);
    {
        // realloc causes there to only be one write to deallocate.
        ASSERT_EQ(state.get_dealloc_writer().num_writes, 0);
        int last_size = 0;
        // Allocate vector<int>
        {
            ert::vector<int> v;
            for (int i = 0; i < 100; i++)
                increase_and_check(v);
            last_size = state.get_alloc_writer().latest_record.size;
        }
        // The memory gets deallocated everytime.
        ASSERT_GE(state.get_dealloc_writer().latest_record.size, last_size);
    }
    {
        int last_size = 0;
        // Allocate vector<int>
        {
            char buffer[1024]{};
            std::pmr::monotonic_buffer_resource rsrc{std::data(buffer), std::size(buffer)};
            ert::pmr::vector<int> v(&rsrc);
            for (int i = 0; i < 100; i++)
                increase_and_check(v);
            last_size = state.get_alloc_writer().latest_record.size;
        }
        ASSERT_GE(state.get_dealloc_writer().latest_record.size, last_size);
    }
}

TEST_F(stl_container_tests, allocate_lists) {
    auto &state = ert::profile_state<ert::default_writer_type>::get_state();

    auto increase_and_check = [&] < typename ListType > (ListType & v, int
    pre_calc_size) {
        v.push_back(1);
        auto alloc_array_record = state.get_alloc_writer().latest_record;
        // Because we don't know the growth ratio of vectors, we can't make
        // simple assumptions to use ASSERT_EQ even though 2^n seems to be
        // the common implementation right now.
        ASSERT_EQ(alloc_array_record.size, pre_calc_size);
    };

    // The writer must've been set up at the beginning of the program.
    ASSERT_EQ(state.get_alloc_writer().is_setup, true);
    {
        int last_size = 0;
        auto expected_size = 0;
        // Allocate vector<int>
        {
            ert::list<int> v;
            v.push_back(1);
            auto alloc_array_record = state.get_alloc_writer().latest_record;
            expected_size = alloc_array_record.size;
            for (int i = 0; i < 100; i++) {
                ASSERT_EQ(state.get_alloc_writer().num_writes, i + 1);
                increase_and_check(v, expected_size);
            }
            ASSERT_EQ(state.get_dealloc_writer().num_writes, 0);
        }
        // We inserted 101 elements.
        ASSERT_EQ(state.get_dealloc_writer().num_writes, 101);
        ASSERT_GE(state.get_dealloc_writer().latest_record.size, expected_size);
    }
    {
        int last_size = 0;
        auto expected_size = 0;
        // Allocate pmr::vector<int>
        {
            char buffer[30000]{};
            ert::pmr::monotonic_buffer_resource rsrc{std::data(buffer), std::size(buffer)};
            ert::pmr::list<int> v(&rsrc);
            v.push_back(1);
            auto alloc_array_record = state.get_alloc_writer().latest_record;
            expected_size = alloc_array_record.size;
            for (int i = 0; i < 100; i++) {
                // We already inserted 101 elements.
                ASSERT_EQ(state.get_alloc_writer().num_writes, i + 1 + 101);
                increase_and_check(v, expected_size);
            }
        }
        // We already inserted 101 elements. We then inserted 101 elements.
        ASSERT_EQ(state.get_dealloc_writer().num_writes, 101 + 101);
        ASSERT_GE(state.get_dealloc_writer().latest_record.size, expected_size);
    }
}

TEST_F(stl_container_tests, allocate_maps) {
    auto &state = ert::profile_state<ert::default_writer_type>::get_state();

    int counter = 1;
    auto increase_and_check = [&] < typename MapType > (MapType & m, int pre_calc_size) {
        m.emplace(counter, counter);
        auto alloc_array_record = state.get_alloc_writer().latest_record;
        // Because we don't know the growth ratio of vectors, we can't make
        // simple assumptions to use ASSERT_EQ even though 2^n seems to be
        // the common implementation right now.
        ASSERT_EQ(alloc_array_record.size, pre_calc_size);
        counter++;
    };

    // The writer must've been set up at the beginning of the program.
    ASSERT_EQ(state.get_alloc_writer().is_setup, true);
    {
        int last_size = 0;
        auto expected_size = 0;
        // Allocate map<int>
        {
            ert::map<int, int> m;
            m.emplace(counter, counter);
            counter++;

            auto alloc_array_record = state.get_alloc_writer().latest_record;
            expected_size = alloc_array_record.size;
            for (int i = 0; i < 100; i++) {
                ASSERT_EQ(state.get_alloc_writer().num_writes, i + 1);
                increase_and_check(m, expected_size);
            }
            ASSERT_EQ(state.get_dealloc_writer().num_writes, 0);
        }
        // We inserted 101 elements.
        ASSERT_EQ(state.get_dealloc_writer().num_writes, 101);
        ASSERT_GE(state.get_dealloc_writer().latest_record.size, expected_size);
    }
    {
        int last_size = 0;
        auto expected_size = 0;
        // Allocate pmr::map<int>
        {
            char buffer[30000]{};
            ert::pmr::monotonic_buffer_resource rsrc{std::data(buffer), std::size(buffer)};
            ert::pmr::map<int, int> m(&rsrc);
            m.emplace(counter, counter);
            counter++;

            auto alloc_array_record = state.get_alloc_writer().latest_record;
            expected_size = alloc_array_record.size;
            for (int i = 0; i < 100; i++) {
                // We already inserted 101 elements.
                ASSERT_EQ(state.get_alloc_writer().num_writes, i + 1 + 101);
                increase_and_check(m, expected_size);
            }
        }
        // We already inserted 101 elements. We then inserted 101 elements.
        ASSERT_EQ(state.get_dealloc_writer().num_writes, 101 + 101);
        ASSERT_GE(state.get_dealloc_writer().latest_record.size, expected_size);
    }
}

TEST_F(stl_container_tests, instantiate_all_stl_containers) {
    { ert::array<int, 10> _a {}; }
    { ert::vector<int> _a {}; }
    { ert::deque<int> _a {}; }
    { ert::forward_list<int> _a {}; }
    { ert::list<int> _a {}; }
    { ert::set<int> _a {}; }
    { ert::multiset<int> _a {}; }
    { ert::map<int, int> _a {}; }
    { ert::multimap<int, int> _a {}; }
    { ert::unordered_set<int> _a {}; }
    { ert::unordered_multiset<int> _a {}; }
    { ert::unordered_map<int, int> _a {}; }
    { ert::unordered_multimap <int, int> _a {}; }
    { ert::stack<int> _a {}; }
    { ert::queue<int> _a {}; }
    { ert::string _a {}; }
    { ert::wstring _a {}; }
    { ert::u8string _a {}; }
    { ert::u16string _a {}; }
    { ert::u32string _a {}; }

    // Explicitly pass in the allocator
    { ert::array<int, 10> _a {}; }
    { ert::vector<int, test_allocator<int>> _a {}; }
    { ert::deque<int, test_allocator<int>> _a {}; }
    { ert::forward_list<int, test_allocator<int>> _a {}; }
    { ert::list<int, test_allocator<int>> _a {}; }
    { ert::set<int, std::less<int>, test_allocator<int>> _a {}; }
    { ert::multiset<int, std::less<int>, test_allocator<int>> _a {}; }
    { ert::map<int, int, std::less<int>, test_allocator<std::pair<const int, int>>> _a {}; }
    { ert::multimap<int, int, std::less<int>, test_allocator<std::pair<const int, int>>> _a {}; }
    { ert::unordered_set<int, std::hash<int>, test_allocator<int>> _a {}; }
    { ert::unordered_multiset<int, std::hash<int>, test_allocator<int>> _a {}; }
    { ert::unordered_map<int, int, std::hash<int>, test_allocator<std::pair<const int, int>>> _a {}; }
    { ert::unordered_multimap <int, int, std::hash<int>, test_allocator<std::pair<const int, int>>> _a {}; }
    { ert::stack<int, ert::deque<int, test_allocator<int>>> _a {}; }
    { ert::queue<int, ert::deque<int, test_allocator<int>>> _a {}; }
}

TEST_F(stl_container_tests, instantiate_pmr_stl_containers) {
    { ert::pmr::vector<int> _a {}; }
    { ert::pmr::deque<int> _a {}; }
    { ert::pmr::forward_list<int> _a {}; }
    { ert::pmr::list<int> _a {}; }
    { ert::pmr::set<int> _a {}; }
    { ert::pmr::multiset<int> _a {}; }
    { ert::pmr::map<int, int> _a {}; }
    { ert::pmr::multimap<int, int> _a {}; }
    { ert::pmr::unordered_set<int> _a {}; }
    { ert::pmr::unordered_multiset<int> _a {}; }
    { ert::pmr::unordered_map<int, int> _a {}; }
    { ert::pmr::unordered_multimap <int, int> _a {}; }
    { ert::pmr::string _a {}; }
    { ert::pmr::wstring _a {}; }
    { ert::pmr::u8string _a {}; }
    { ert::pmr::u16string _a {}; }
    { ert::pmr::u32string _a {}; }
}
// ===

// === Smart pointer tests ===
TEST_F(smart_pointer_tests, shared_ptr) {
    auto& state = ert::profile_state<ert::default_writer_type>::get_state();
    // The writer must've been set up at the beginning of the program.
    ASSERT_EQ(state.get_alloc_writer().is_setup, true);
    // We didn't allocate yet
    ASSERT_EQ(state.get_alloc_writer().num_writes, 0);
    ASSERT_EQ(state.get_dealloc_writer().num_writes, 0);
    {
        ert::shared_ptr<int> ptr = ert::make_shared<int>();
        ASSERT_EQ(state.get_alloc_writer().num_writes, 1);
        ASSERT_EQ(state.get_dealloc_writer().num_writes, 0);

        auto alloc_array_record = state.get_alloc_writer().latest_record;
        // The size needs to be AS big as int, and likely bigger because shared_ptr initializes some of its own
        // data on the heap.
        ASSERT_GE(alloc_array_record.size, sizeof(int));
    }
    ASSERT_EQ(state.get_alloc_writer().num_writes, 1);
    ASSERT_EQ(state.get_dealloc_writer().num_writes, 1);
    {
        // Shared ptr has type erasure
        ert::shared_ptr<int> ptr = ert::allocate_shared<int>(test_allocator<int>());
        ASSERT_EQ(state.get_alloc_writer().num_writes, 2);
        ASSERT_EQ(state.get_dealloc_writer().num_writes, 1);

        auto alloc_array_record = state.get_alloc_writer().latest_record;
        // The size needs to be AS big as int, and likely bigger because shared_ptr initializes some of its own
        // data on the heap.
        ASSERT_GE(alloc_array_record.size, sizeof(int));
    }
    ASSERT_EQ(state.get_alloc_writer().num_writes, 2);
    ASSERT_EQ(state.get_dealloc_writer().num_writes, 2);
}

TEST_F(smart_pointer_tests, unique_ptr) {
    auto& state = ert::profile_state<ert::default_writer_type>::get_state();
    // The writer must've been set up at the beginning of the program.
    ASSERT_EQ(state.get_alloc_writer().is_setup, true);
    // We didn't allocate yet
    ASSERT_EQ(state.get_alloc_writer().num_writes, 0);
    ASSERT_EQ(state.get_dealloc_writer().num_writes, 0);
    {
        ert::unique_ptr<int> ptr = ert::make_unique<int>();
        ASSERT_EQ(state.get_alloc_writer().num_writes, 1);
        ASSERT_EQ(state.get_dealloc_writer().num_writes, 0);

        auto alloc_array_record = state.get_alloc_writer().latest_record;
        // The size needs to be AS big as int, and likely bigger because shared_ptr initializes some of its own
        // data on the heap.
        ASSERT_GE(alloc_array_record.size, sizeof(int));
    }
    ASSERT_EQ(state.get_alloc_writer().num_writes, 1);
    ASSERT_EQ(state.get_dealloc_writer().num_writes, 1);
    {
        // This type is kinda long
        auto ptr = ert::allocate_unique<int>(test_allocator<inert::unique_ptr<T, details::alloc_deleter>t>());
        ASSERT_EQ(state.get_alloc_writer().num_writes, 2);
        ASSERT_EQ(state.get_dealloc_writer().num_writes, 1);

        auto alloc_array_record = state.get_alloc_writer().latest_record;
        // The size needs to be AS big as int, and likely bigger because shared_ptr initializes some of its own
        // data on the heap.
        ASSERT_GE(alloc_array_record.size, sizeof(int));
    }
    ASSERT_EQ(state.get_alloc_writer().num_writes, 2);
    ASSERT_EQ(state.get_dealloc_writer().num_writes, 2);
}
