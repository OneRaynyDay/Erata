#pragma once
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

#include <chrono>
#include <functional>
#include <typeinfo>
#include <cxxabi.h>

// Using spdlog to write to files
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

// Using json to stringify ending maps
#include "nlohmann/json.hpp"

// Using fmt to format record structs into json
#include "spdlog/fmt/fmt.h"
#include "spdlog/fmt/bundled/chrono.h"

// TODO remove:
#include <iostream>

#include <memory_resource>

namespace ert {

using json = nlohmann::json;

// Keep this many records in the buffer
static constexpr int record_buffer_size = 100;
// Designate the global scope to be of hash 0.
static constexpr auto GLOBAL_SCOPE_HASH = 0;

const std::string alloc_filename("todo_change_this_alloc.txt");
const std::string dealloc_filename("todo_change_this_dealloc.txt");

/// Statically managed state objects for allocators which are trivially copyable and movable.
/// TODO: Make this thread-safe. This may require a singleton-per-thread thread_local storage design.
class profile_state {
    /// We need timestamp for both record and the beginning of the program.
    using timestamp_type = unsigned long;
    /// Record type used in each memory transaction
    /// TODO: Optimize using alignas
    struct record {
        using location_type = unsigned long;
        static_assert(sizeof(location_type) >= sizeof(void*), "Address of variables cannot be stored in current location type.");

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

        template <typename T>
        record(std::size_t scope_hash, std::size_t size, T* ptr) noexcept :
            scope_hash(scope_hash), type_hash(typeid(T).hash_code()), size(size), location((location_type) ptr),
            timestamp(get_current_timestamp()) {}

        record() = default;
        record(const record&) = default;
        record& operator=(const record&) = default;
    };

    /// Aggregation types
    /// averages can be computed with a tuple of (double, unsigned int),
    /// and stddev can be computed with the square of sum aggregates also in (double, unsigned int)
    /// currently we calculate up to the 2nd moment.
    using moment_type = std::tuple<std::size_t, std::size_t>;
    /// maxes can be computed with a single type std::size_t
    using max_type = std::size_t;
    /// mins similarly
    using min_type = std::size_t;
    /// count can be represented as unsigned int
    using count_type = std::size_t;

    // Aggregates computed in O(1) time
    moment_type moments;
    max_type max_size;
    min_type min_size;
    count_type counts;

    /// fields used for processing, formatting and writing data.

    // Logger type is single-threaded for performance. We copy a new
    // single-threaded logger upon creating a new thread.
    using logger_type = std::shared_ptr<spdlog::logger>;

    // TODO Optimize?
    // The user is allows to change the scope of the profile_state so they can easily visualize which
    // section of the code they're currently running. We take the scope name and hash it here.
    // We'll be writing to disk a lot, so we don't want to serialize a ton of strings, but rather size_t's.
    std::stack<std::size_t> scopes;
    std::unordered_map<std::size_t, std::string> scope_names;

    // TODO Optimize?
    // Storing the cxx abi name mangling dictionary information here. Similar to scope names, we want to
    // keep the type_names hashed so we write less to disk.
    std::unordered_map<std::size_t, std::string> type_names;

    // Logger object associated with the profile_state
    logger_type alloc_logger;
    logger_type dealloc_logger;

    // We need the beginning of the program to compare
    timestamp_type start_time;
public:
    // Following clang-tidy's rules to move this to public
    // Disallow copying/assigning/moving actual resources
    // The standard specifies that copy ctor/assignment explicitly declared
    // prevents implicit move constructors from being created.
    profile_state(const profile_state&) = delete;
    profile_state& operator=(const profile_state&) = delete;

    /// Return the singleton profile_state.
    static profile_state& get_state() noexcept {
        /// Singleton containing state necessary for all profile_allocators
        static profile_state state;
        return state;
    }

    /// Push a string context onto the existing scope. This is useful for
    /// Isolating parts of subroutines where extensive dynamic allocation
    /// is being done. In the final output, we should expect the allocations
    /// to be recorded with this tag.
    void push_scope(const std::string& s) {
        auto hash = std::hash<std::string>{}(s);
        scope_names.emplace(hash, s);
        scopes.push(hash);
    }

    /// Pops the scope from the stack and returns the context to the user.
    std::string pop_scope() {
        auto hash = scopes.top();
        scopes.pop();
        auto it = scope_names.find(hash);
        if (it == scope_names.end())
            throw std::runtime_error("Attempted to pop a scope that was never entered. This should never happen.");
        return it->second;
    }

    template <typename T>
    void record_allocation(T* p, std::size_t n) {
        // Updating statistics
        auto alloc_size = sizeof(T) * n;
        max_size = std::max(max_size, alloc_size);
        min_size = std::min(min_size, alloc_size);
        counts++;
        std::get<0>(moments) += alloc_size;
        std::get<1>(moments) += alloc_size * alloc_size;

        auto hash_code = typeid(T).hash_code();
        if (type_names.find(hash_code) == type_names.end()) [[unlikely]]
            type_names.emplace(hash_code, get_demangled_name<T>());

        alloc_logger->info(record(scopes.top(), alloc_size, p));
    }

    template <typename T>
    void record_deallocation(T* p, std::size_t size) {
        auto alloc_size = sizeof(T) * size;
        // We have to deal with the case when two different threads are working in their own type name maps
        // and allocating/deallocating things from each other's routines.
        // Then we need to populate the hash codes in here as well.
        auto hash_code = typeid(T).hash_code();
        if (type_names.find(hash_code) == type_names.end()) [[unlikely]]
                    type_names.emplace(hash_code, get_demangled_name<T>());

        dealloc_logger->info(record(scopes.top(), alloc_size, p));
    }

private:
    profile_state() noexcept : moments(0, 0), max_size(0), min_size(0), counts(0),
                               scopes(), start_time(get_current_timestamp()) {
        scopes.push(GLOBAL_SCOPE_HASH);
        setup_logger(alloc_logger, "alloc_file_logger_todo", alloc_filename);
        setup_logger(dealloc_logger, "dealloc_file_logger_todo", dealloc_filename);
    }

    ~profile_state() {
        // We write the symbols into the given file names since we don't want to risk throwing in the destructor
        // by writing to another file that may already exist.
        // We utilize the JSON library here.
        json json_scope_map(scope_names);
        json json_type_map(type_names);
        std::string scope_dump = json_scope_map.dump();
        std::string type_dump = json_type_map.dump();
        end_logger(alloc_logger, scope_dump, type_dump);
        end_logger(dealloc_logger, scope_dump, type_dump);
    }

    void setup_logger(logger_type& logger, const std::string& logger_name, const std::string& file_name) {
        logger = spdlog::basic_logger_st(logger_name, file_name);
        logger->set_pattern("%v");
        logger->info("{ \"values\": [");
    }

    void end_logger(logger_type& logger, const std::string& scope_json, const std::string& type_json) {
        logger->info("], ");
        logger->info("\"scopes\": {},", scope_json);
        logger->info("\"types\": {},", type_json);
        logger->info("\"start_ts\": {},", start_time);
        logger->info("}");
        logger->flush();
    }

    template <typename T>
    std::string get_demangled_name() {
        // Add string type id to map if it doesn't exist.
#ifdef __GNUG__
        // Setting it to a value that the fn cannot return
        int status = -4;
        char* res = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status);
        std::string s(res);
        // res is allocated on the heap from above.
        std::free(res);
        return s;
#else
        throw std::runtime_error("Not implemented for non-g++ platform");
#endif
    }

    static timestamp_type get_current_timestamp() {
        return std::chrono::system_clock::now().time_since_epoch().count();
    }
};

// Exposing the functions to ert:: namespace for ease of use.
void push_scope(const std::string& s) {
    profile_state::get_state().push_scope(s);
}

std::string pop_scope() {
    return profile_state::get_state().pop_scope();
}

template<typename T, typename base_allocator=std::allocator<T>>
class profile_allocator {
public:
    /// Necessary for allocators, propagate exactly what the base_allocator
    /// wants.
    typedef typename std::allocator_traits<base_allocator>::size_type size_type;
    typedef typename std::allocator_traits<base_allocator>::difference_type difference_type;
    typedef typename std::allocator_traits<base_allocator>::pointer pointer;
    typedef typename std::allocator_traits<base_allocator>::const_pointer const_pointer;
    typedef typename std::allocator_traits<base_allocator>::value_type value_type;

    // NOTE: Allocator traits has no reference customization point. Anything that uses
    // profile allocator most likely will no longer use the two fields below either:
    // reference;
    // const_reference;
    //
    // If there really is a special case that requires customizing reference types,
    // then come back to this later.

    template <class U>
    struct rebind {
        typedef profile_allocator<U,
                                  typename std::allocator_traits<base_allocator>::template rebind_alloc<U>> other;
    };

    /// Forward all necessary arguments to the nested allocator.
    template <typename... Args>
    profile_allocator(Args&&... args) noexcept : alloc(std::forward<Args>(args)...), state(profile_state::get_state()){}

    /// Upon copy construction, we must guarrantee that it must be the base allocator
    /// but we allow it to be any type U templatized by base_allocator. To do this we
    /// unfortunately need to use rebind_alloc.
    /// In addition, to do this, we need to make other profile_allocators friends, since different template instantiations
    /// cannot access each others' private members.
    template <typename U, typename A> friend class profile_allocator;
    template <typename U>
    profile_allocator(
        const profile_allocator<U, typename std::allocator_traits<base_allocator>::template rebind_alloc<U>>& other) noexcept :
            alloc(other.alloc), state(profile_state::get_state()) {}

    [[nodiscard]] T* allocate(std::size_t n) {
        T* p = alloc.allocate(n);
        state.record_allocation(p, n);
        return p;
    }

    void deallocate(T* p, std::size_t size) noexcept {
        alloc.deallocate(p, size);
        state.record_deallocation(p, size);
    }

private:
    base_allocator alloc;
    profile_state& state;
    // We want to call the underlying allocator type
};

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

// Populating fmt with necessary parsers for records
namespace fmt {
template<>
struct formatter<::ert::profile_state::record> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const ::ert::profile_state::record &number, FormatContext &ctx) {
        return format_to(ctx.out(),
                "{{ \"ts\":{0}, \"sh\": {1}, \"th\": {2}, \"s\": {3}, \"l\": {4} }},",
                number.timestamp, number.scope_hash, number.type_hash, number.size, number.location);
    }
};
}

