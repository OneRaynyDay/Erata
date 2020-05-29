#pragma once
#include "alloc.hpp"

namespace ert {
/// Offer equivalent STL container types.
template <typename T, typename Alloc=std::allocator<T>>
using vector = std::vector<T, profile_allocator<T, Alloc>>;
template <typename T, typename Alloc=std::allocator<T>>
using deque = std::deque<T, profile_allocator<T, Alloc>>;
template <typename T, typename Alloc=std::allocator<T>>
using forward_list = std::forward_list<T, profile_allocator<T, Alloc>>;
template <typename T, typename Alloc=std::allocator<T>>
using list = std::list<T, profile_allocator<T, Alloc>>;
template <typename T, typename Alloc=std::allocator<T>>
using set = std::set<T, profile_allocator<T, Alloc>>;
template <typename T, typename Alloc=std::allocator<T>>
using map = std::map<T, profile_allocator<T, Alloc>>;
template <typename T, typename Alloc=std::allocator<T>>
using multiset = std::multiset<T, profile_allocator<T, Alloc>>;
template <typename T, typename Alloc=std::allocator<T>>
using multimap = std::multimap<T, profile_allocator<T, Alloc>>;
template <typename T, typename Alloc=std::allocator<T>>
using unordered_set = std::unordered_set<T, profile_allocator<T, Alloc>>;
template <typename T, typename Alloc=std::allocator<T>>
using unordered_map = std::unordered_map<T, profile_allocator<T, Alloc>>;
template <typename T, typename Alloc=std::allocator<T>>
using unordered_multiset = std::unordered_multiset<T, profile_allocator<T, Alloc>>;
template <typename T, typename Alloc=std::allocator<T>>
using unordered_multimap = std::unordered_multimap<T, profile_allocator<T, Alloc>>;
template <typename T, typename Alloc=std::allocator<T>>
using stack = std::stack<T, profile_allocator<T, Alloc>>;
template <typename T, typename Alloc=std::allocator<T>>
using queue = std::queue<T, profile_allocator<T, Alloc>>;
template <typename CharT, class Traits = std::char_traits<CharT>, class Alloc = std::allocator<CharT>>
using basic_string = std::basic_string<CharT, Traits, profile_allocator<CharT, Alloc>>;
using string = basic_string<char>;
using wstring = basic_string<wchar_t>;
using u8string = basic_string<char8_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;

namespace pmr {
template <typename T>
using vector = vector<T, std::pmr::polymorphic_allocator<T>>;
template <typename T>
using deque = deque<T, std::pmr::polymorphic_allocator<T>>;
template <typename T>
using forward_list = forward_list<T, std::pmr::polymorphic_allocator<T>>;
template <typename T>
using list = list<T, std::pmr::polymorphic_allocator<T>>;
template <typename T>
using set = set<T, std::pmr::polymorphic_allocator<T>>;
template <typename T>
using map = map<T, std::pmr::polymorphic_allocator<T>>;
template <typename T>
using multiset = multiset<T, std::pmr::polymorphic_allocator<T>>;
template <typename T>
using multimap = multimap<T, std::pmr::polymorphic_allocator<T>>;
template <typename T>
using unordered_set = unordered_set<T, std::pmr::polymorphic_allocator<T>>;
template <typename T>
using unordered_map = unordered_map<T, std::pmr::polymorphic_allocator<T>>;
template <typename T>
using unordered_multiset = unordered_multiset<T, std::pmr::polymorphic_allocator<T>>;
template <typename T>
using unordered_multimap = unordered_multimap<T, std::pmr::polymorphic_allocator<T>>;
template <typename T>
using stack = stack<T, std::pmr::polymorphic_allocator<T>>;
template <typename T>
using queue = queue<T, std::pmr::polymorphic_allocator<T>>;
template <typename CharT, class Traits = std::char_traits<CharT>>
using basic_string = basic_string<CharT, Traits, std::pmr::polymorphic_allocator<CharT>>;
using string = basic_string<char>;
using wstring = basic_string<wchar_t>;
using u8string = basic_string<char8_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;
} // namespace pmr
} // namespace erata
