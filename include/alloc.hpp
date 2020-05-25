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

#include <memory_resource>

namespace erata {

// Keep this many records in the buffer
static constexpr int record_buffer_size = 100;
// Global namespace for now when we don't deal w/ thread safety.
static constexpr auto GLOBAL_SCOPE = "global";

/// Statically managed state objects for allocators which are trivially copyable and movable.
/// TODO: Make this thread-safe. This may require a singleton-per-thread thread_local storage design.
class profile_state {
    /// Record type
    /// TODO: Optimize using alignas
    struct record {
        std::string scope;
        std::size_t size;
        void* location;
    };

    /// Aggregation types
    /// averages can be computed with a tuple of (double, unsigned int),
    /// and stddev can be computed with the square of sum aggregates also in (double, unsigned int)
    /// currently we calculate up to the 2nd moment.
    using moment_type = std::tuple<double, double, unsigned int>;
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
    count_type count;

    // The user is allows to change the scope of the profile_state so they can easily visualize which
    // section of the code they're currently running.
    std::stack<std::string> scopes;

    // Book-keeping for maximum number of records.
    std::array<record, record_buffer_size> record_buffer;
    int record_buffer_counter;

public:
    // Following clang-tidy's rules to move this to public
    // Disallow copying/assigning/moving actual resources
    // The standard specifies that copy ctor/assignment explicitly declared
    // prevents implicit move constructors from being created.
    profile_state(const profile_state&) = delete;
    profile_state& operator=(const profile_state&) = delete;

    /// Return the singleton profile_state.
    static profile_state& get_state() {
        /// Singleton containing state necessary for all profile_allocators
        static profile_state state;
        return state;
    }

    /// Push a string context onto the existing scope. This is useful for
    /// Isolating parts of subroutines where extensive dynamic allocation
    /// is being done. In the final output, we should expect the allocations
    /// to be recorded with this tag.
    static void push_scope(const std::string& s) {
        get_state().scopes.push(s);
    }

    /// Pops the scope from the stack and returns the context to the user.
    static std::string pop_scope() {
        auto s = get_state().scopes.top();
        get_state().scopes.pop();
        return s;
    }

private:
    profile_state() : moments(0, 0, 0), max_size(0), min_size(0), count(0),
                      scopes(), record_buffer(), record_buffer_counter(0) {
        scopes.push(GLOBAL_SCOPE);
    }
};

// Exposing the functions to ert:: namespace for ease of use.
static constexpr auto push_scope = &profile_state::push_scope;
static constexpr auto pop_scope = &profile_state::pop_scope;

template<typename T, typename base_allocator=std::allocator<T>>
class profile_allocator {
public:
    /// Necessary for allocators, propagate exactly what the base_allocator
    /// wants.
    typedef typename std::allocator_traits<base_allocator>::size_type size_type;
    typedef typename std::allocator_traits<base_allocator>::difference_type difference_type;
    typedef typename std::allocator_traits<base_allocator>::pointer pointer_type;
    typedef typename std::allocator_traits<base_allocator>::const_pointer const_pointer;
    typedef typename std::allocator_traits<base_allocator>::reference reference;
    typedef typename std::allocator_traits<base_allocator>::const_reference const_reference;
    typedef typename std::allocator_traits<base_allocator>::value_type value_type;

    template <class U>
    struct rebind {
        typedef profile_allocator<U,
                                  typename std::allocator_traits<base_allocator>::template rebind_alloc<U>> other;
    };

    /// Forward all necessary arguments to the nested allocator.
    template <typename... Args>
    profile_allocator(Args&&... args) noexcept : alloc(std::forward<Args>(args)...) {}

    /// Upon copy construction, we must guarrantee that it must be the base allocator
    /// but we allow it to be any type U templatized by base_allocator. To do this we
    /// unfortunately need to use rebind_alloc.
    template <typename U>
    constexpr profile_allocator(const profile_allocator<U,
            typename std::allocator_traits<base_allocator>::template rebind_alloc<U>>&) noexcept {}

    [[nodiscard]] T* allocate(std::size_t n) {
        return alloc.allocate(n);
    }

    void deallocate(T* p, std::size_t size) noexcept {
        alloc.deallocate(p, size);
    }

private:
    base_allocator alloc;
    profile_state& state;
    // We want to call the underlying allocator type
};

/// Offer equivalent STL container types.
template<typename T, typename Alloc=std::allocator<T>>
using vector = std::vector<T, profile_allocator<T, Alloc>>;

template<typename T, typename Alloc=std::allocator<T>>
using deque = std::deque<T, profile_allocator<T, Alloc>>;

template<typename T, typename Alloc=std::allocator<T>>
using forward_list = std::forward_list<T, profile_allocator<T, Alloc>>;

template<typename T, typename Alloc=std::allocator<T>>
using list = std::list<T, profile_allocator<T, Alloc>>;

template<typename T, typename Alloc=std::allocator<T>>
using set = std::set<T, profile_allocator<T, Alloc>>;

template<typename T, typename Alloc=std::allocator<T>>
using map = std::map<T, profile_allocator<T, Alloc>>;

template<typename T, typename Alloc=std::allocator<T>>
using multiset = std::multiset<T, profile_allocator<T, Alloc>>;

template<typename T, typename Alloc=std::allocator<T>>
using multimap = std::multimap<T, profile_allocator<T, Alloc>>;

template<typename T, typename Alloc=std::allocator<T>>
using unordered_set = std::unordered_set<T, profile_allocator<T, Alloc>>;

template<typename T, typename Alloc=std::allocator<T>>
using unordered_map = std::unordered_map<T, profile_allocator<T, Alloc>>;

template<typename T, typename Alloc=std::allocator<T>>
using unordered_multiset = std::unordered_multiset<T, profile_allocator<T, Alloc>>;

template<typename T, typename Alloc=std::allocator<T>>
using unordered_multimap = std::unordered_multimap<T, profile_allocator<T, Alloc>>;

template<typename T, typename Alloc=std::allocator<T>>
using stack = std::stack<T, profile_allocator<T, Alloc>>;

template<typename T, typename Alloc=std::allocator<T>>
using queue = std::queue<T, profile_allocator<T, Alloc>>;

} // namespace erata