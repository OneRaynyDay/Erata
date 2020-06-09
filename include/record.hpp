#pragma once
#include <chrono>
#include "types.hpp"

namespace ert {

inline timestamp_type get_current_timestamp() {
    return std::chrono::system_clock::now().time_since_epoch().count();
}

// TODO alignas?
struct record {
    static_assert(sizeof(location_type) >= sizeof(void *),
                  "Address of variables cannot be stored in current location type.");
    // The hashed name of the scope in which this variable is allocated.
    std::size_t scope_hash;
    // The hashed type of object allocated/deallocated.
    std::size_t type_hash;
    // The size of object allocated/deallocated.
    std::size_t size;
    // The address in the heap.
    location_type location;
    // Timestamp of the struct since creation
    timestamp_type timestamp;

    template<typename T>
    record(std::size_t scope_hash, std::size_t size, T *ptr) noexcept :
           scope_hash(scope_hash), type_hash(typeid(T).hash_code()), size(size), location((location_type) ptr),
           timestamp(get_current_timestamp()) {}

    record() = default;
    record(const record &) = default;
    record &operator=(const record &) = default;
};
} // namespace ert
