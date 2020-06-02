#pragma once

#include "record.hpp"

#include "writer_concept.hpp"

namespace ert {
namespace writer {

struct test_writer {
    bool is_setup;
    int num_writes;
    bool is_end;

    test_writer() : is_setup(false), num_writes(0), is_end(false) {}

    void setup(const std::string& writer_name, const std::string& file_name) {
        // No-op
        is_setup = true;
    }

    void write(const record& record) const {
        num_writes++;
    }

    void end(const timestamp_type& ts, const scope_map& scope_names, const type_map& type_names) const {
        is_end = true;
    }
};

} // namespace writer
} // namespace ert