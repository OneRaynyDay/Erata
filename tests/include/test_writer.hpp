#pragma once

#include "record.hpp"

#include "writer_concept.hpp"

namespace ert {
namespace writer {

struct test_writer {
    bool is_setup;
    int num_writes;
    bool is_end;

    record latest_record;

    test_writer() : is_setup(false), num_writes(0), is_end(false) {}
    test_writer(const test_writer&) = default;
    test_writer& operator=(const test_writer&) = default;

    void setup(const std::string& writer_name) {
        // No-op
        is_setup = true;
    }

    void write(const record& record) {
        num_writes++;
        latest_record = record;
    }

    void end(const timestamp_type& ts, const scope_map& scope_names, const type_map& type_names) {
        is_end = true;
    }
};

} // namespace writer
} // namespace ert