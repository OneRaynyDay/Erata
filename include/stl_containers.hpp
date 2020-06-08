#pragma once
#include "alloc.hpp"
// Including all the data structures here so we can typedef in pmr namespace
#include <array>
#include <vector>
#include <deque>
#include <forward_list>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>
#include <string>


namespace ert {
/// Offer equivalent STL container types.

// Array is always on the stack:
// https://stackoverflow.com/questions/39548254/does-stdarray-guarantee-allocation-on-the-stack-only
template <typename T, std::size_t N>
using array = std::array<T, N>;

template <typename T, typename Alloc=std::allocator<T>>
using vector = std::vector<T, profile_allocator<T, Alloc>>;
template <typename T, typename Alloc=std::allocator<T>>
using deque = std::deque<T, profile_allocator<T, Alloc>>;
template <typename T, typename Alloc=std::allocator<T>>
using forward_list = std::forward_list<T, profile_allocator<T, Alloc>>;
template <typename T, typename Alloc=std::allocator<T>>
using list = std::list<T, profile_allocator<T, Alloc>>;

template <typename K, typename Compare=std::less<K>, typename Alloc=std::allocator<K>>
using set = std::set<K, Compare, profile_allocator<K, Alloc>>;
template <typename K, typename Compare=std::less<K>, typename Alloc=std::allocator<K>>
using multiset = std::multiset<K, Compare, profile_allocator<K, Alloc>>;

template <typename K, typename T, typename Compare=std::less<K>, typename Alloc=std::allocator<std::pair<const K, T>>>
using map = std::map<K, T, Compare, profile_allocator<std::pair<const K, T>, Alloc>>;
template <typename K, typename T, typename Compare=std::less<K>, typename Alloc=std::allocator<std::pair<const K, T>>>
using multimap = std::multimap<K, T, Compare, profile_allocator<std::pair<const K, T>, Alloc>>;

template <typename K, typename Hash=std::hash<K>, typename KeyEqual=std::equal_to<K>, typename Alloc=std::allocator<K>>
using unordered_set = std::unordered_set<K, Hash, KeyEqual, profile_allocator<K, Alloc>>;
template <typename K, typename Hash=std::hash<K>, typename KeyEqual=std::equal_to<K>, typename Alloc=std::allocator<K>>
using unordered_multiset = std::unordered_multiset<K, Hash, KeyEqual, profile_allocator<K, Alloc>>;

template <typename K, typename T, typename Hash=std::hash<K>, typename KeyEqual=std::equal_to<K>, typename Alloc=std::allocator<std::pair<const K, T>>>
using unordered_map = std::unordered_map<K, T, Hash, KeyEqual, profile_allocator<std::pair<K, T>, Alloc>>;
template <typename K, typename T, typename Hash=std::hash<K>, typename KeyEqual=std::equal_to<K>, typename Alloc=std::allocator<std::pair<const K, T>>>
using unordered_multimap = std::unordered_multimap<K, T, Hash, KeyEqual, profile_allocator<std::pair<K, T>, Alloc>>;

// We'll be using deque as default, and it's bound to profile_allocator.
template <typename T, typename Container=deque<T>>
using stack = std::stack<T, Container>;
template <typename T, typename Container=deque<T>>
using queue = std::queue<T, Container>;

template <typename CharT, class Traits = std::char_traits<CharT>, class Alloc = std::allocator<CharT>>
using basic_string = std::basic_string<CharT, Traits, profile_allocator<CharT, Alloc>>;
using string = basic_string<char>;
using wstring = basic_string<wchar_t>;
using u8string = basic_string<char8_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;

namespace pmr {
// In std::pmr, we don't have the following containers:
// - stack
// - queue

// We left polymorphic allocator as itself instead of wrapping with
// profile_allocator<polymorphic_allocator<>> because it's treated like a
// custom allocator. We want to capture all allocators so when we pass in
// ert::vector<int, ert::pmr::allocator<T>> it's not going to be counted as
// profile_allocator<profile_allocator<T>>.
template <typename T>
using polymorphic_allocator = std::pmr::polymorphic_allocator<T>;

using memory_resource = std::pmr::memory_resource;
using pool_options = std::pmr::pool_options;
using monotonic_buffer_resource = std::pmr::monotonic_buffer_resource;
using synchronized_pool_resource = std::pmr::synchronized_pool_resource;
using unsynchronized_pool_resource = std::pmr::unsynchronized_pool_resource;

template <typename T>
using vector = vector<T, std::pmr::polymorphic_allocator<T>>;
template <typename T>
using deque = deque<T, std::pmr::polymorphic_allocator<T>>;
template <typename T>
using forward_list = forward_list<T, std::pmr::polymorphic_allocator<T>>;
template <typename T>
using list = list<T, std::pmr::polymorphic_allocator<T>>;

template <typename K, typename Compare=std::less<K>>
using set = set<K, Compare, std::pmr::polymorphic_allocator<K>>;
template <typename K, typename Compare=std::less<K>>
using multiset = multiset<K, Compare, std::pmr::polymorphic_allocator<K>>;

template <typename K, typename T, typename Compare=std::less<K>>
using map = map<K, T, Compare, std::pmr::polymorphic_allocator<std::pair<const K, T>>>;
template <typename K, typename T, typename Compare=std::less<K>>
using multimap = multimap<K, T, Compare, std::pmr::polymorphic_allocator<std::pair<const K, T>>>;

template <typename K, typename Hash=std::hash<K>, typename KeyEqual=std::equal_to<K>>
using unordered_set = unordered_set<K, Hash, KeyEqual, std::pmr::polymorphic_allocator<K>>;
template <typename K, typename Hash=std::hash<K>, typename KeyEqual=std::equal_to<K>>
using unordered_multiset = unordered_multiset<K, Hash, KeyEqual, std::pmr::polymorphic_allocator<K>>;

template <typename K, typename T, typename Hash=std::hash<K>, typename KeyEqual=std::equal_to<K>>
using unordered_map = unordered_map<K, T, Hash, KeyEqual, std::pmr::polymorphic_allocator<std::pair<const K, T>>>;
template <typename K, typename T, typename Hash=std::hash<K>, typename KeyEqual=std::equal_to<K>>
using unordered_multimap = unordered_multimap<K, T, Hash, KeyEqual, std::pmr::polymorphic_allocator<std::pair<const K, T>>>;

template <typename CharT, class Traits = std::char_traits<CharT>>
using basic_string = basic_string<CharT, Traits, std::pmr::polymorphic_allocator<CharT>>;
using string = basic_string<char>;
using wstring = basic_string<wchar_t>;
using u8string = basic_string<char8_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;
} // namespace pmr
} // namespace erata
