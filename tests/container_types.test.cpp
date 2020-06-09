#include "alloc.hpp"
#include "stl_containers.hpp"
#include "pointer_types.hpp"
#include "gtest/gtest.h"

using ert::details::stack;
using ert::details::map;

class test_data_structures : public testing::Test {
    virtual void SetUp() {
    }
};

TEST_F(test_data_structures, construct_stack) {
    int num_elements = 1000;
    stack<int> s;
    for (int i = num_elements-1; i >= 0; i--) {
        s.push(i);
    }
    stack<int> s1 = s;
    stack<int> s2(s);
    stack<int> _s3(s);
    stack<int> s3(std::move(_s3));
    ASSERT_EQ(_s3.size(), 0);

    for(int i = 0; i < num_elements; i++) {
        ASSERT_EQ(s.top(), i);
        s.pop();
        ASSERT_EQ(s1.top(), i);
        s1.pop();
        ASSERT_EQ(s2.top(), i);
        s2.pop();
        ASSERT_EQ(s3.top(), i);
        s3.pop();
    }
    ASSERT_EQ(s.size(), 0);
    ASSERT_EQ(s1.size(), 0);
    ASSERT_EQ(s2.size(), 0);
    ASSERT_EQ(s3.size(), 0);
}

TEST_F(test_data_structures, construct_map) {

}
