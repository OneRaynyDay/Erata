#pragma once

// Require record type for logger API
#include "record.hpp"

namespace ert {

namespace writer {

/// Concepts required for the writers used by profile_state. This allows writing to network
/// to files, and maybe to other things.
/// TODO: Somehow consolidate this with the API of profile_allocator
template<typename T> concept is_writer = requires(T a,
                                                  std::string str,
                                                  ert::record record,
                                                  timestamp_type timestamp,
                                                  scope_map scope,
                                                  type_map type) {
    // A mandatory setup stage (can be no-op)
    // So far, passes some kind of string tag
    a.setup(str);
    // Must be able to write records
    a.write(record);
    // A mandatory ending stage (can be no-op)
    // So far, passes the program timestamp, scope map, type map.
    a.end(timestamp, scope, type);
};

} // namespace writer

} // namespace ert
