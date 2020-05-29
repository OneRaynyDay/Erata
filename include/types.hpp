#pragma once
#include <string>
#include <unordered_map>

namespace ert {
/// We need timestamp for both record and the beginning of the program.
using timestamp_type = unsigned long;
using location_type = unsigned long;
using scope_map = std::unordered_map<std::size_t, std::string>;
using type_map = std::unordered_map<std::size_t, std::string>;
} // namespace ert
