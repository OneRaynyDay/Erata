#pragma once

#define STRINGIFY_(a) #a
#define STRINGIFY(a) STRINGIFY_(a)
// Require record type object
#ifndef DEFAULT_WRITER_TYPE
#define DEFAULT_WRITER_TYPE spd_file_logger
#endif
// So far, we only include the single hpp required for writer to prevent code bloat.
#include STRINGIFY(DEFAULT_WRITER_TYPE.hpp)

#include <chrono>
#include <functional>
#include <typeinfo>
#include <cxxabi.h>

// Thread id needed
#include <thread>

// TODO remove:
#include <iostream>

// Including PMR resources
#include <memory_resource>

// Using fmt to format record structs into json
#include "spdlog/fmt/fmt.h"

namespace ert {

/// Using macro-defined writer type
using default_writer_type = ert::writer::DEFAULT_WRITER_TYPE;

// Designate the global scope to be of hash 0.
static constexpr auto GLOBAL_SCOPE_HASH = 0;


/// Statically managed state objects for allocators which are trivially copyable and movable.
/// TODO: Check that this is thread-safe. This may require a singleton-per-thread thread_local storage design.
template <typename writer_type=default_writer_type>
class profile_state {
    // TODO Optimize?
    // The user is allows to change the scope of the profile_state so they can easily visualize which
    // section of the code they're currently running. We take the scope name and hash it here.
    // We'll be writing to disk a lot, so we don't want to serialize a ton of strings, but rather size_t's.
    scope_stack scopes;
    scope_map scope_names;

    // TODO Optimize?
    // Storing the cxx abi name mangling dictionary information here. Similar to scope names, we want to
    // keep the type_names hashed so we write less to disk.
    type_map type_names;

    // Writer object associated with the profile_state
    writer_type alloc_writer;
    writer_type dealloc_writer;

    // We need the beginning of the program to compare
    timestamp_type start_time;

public:
    // Following clang-tidy's rules to move this to public
    // Disallow copying/assigning/moving actual resources
    // The standard specifies that copy ctor/assignment explicitly declared
    // prevents implicit move constructors from being created.
    profile_state(const profile_state&) = delete;
    profile_state& operator=(const profile_state&) = delete;

    /// Getters and setters for writers
    // Scott Meyers - avoid returning handles to members
    writer_type get_alloc_writer() { return alloc_writer; }
    writer_type get_dealloc_writer() { return dealloc_writer; }
    void set_alloc_writer(const writer_type& writer) { alloc_writer = writer; }
    void set_dealloc_writer(const writer_type& writer) { dealloc_writer = writer; }

    /// Return the singleton profile_state.
    static profile_state& get_state() noexcept {
        /// Singleton containing state necessary for all profile_allocators
        static thread_local profile_state state;
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
    void pop_scope() {
        scopes.pop();
    }

    template <typename T>
    void record_allocation(T* p, std::size_t n) {
        // Updating statistics
        auto alloc_size = sizeof(T) * n;

        auto hash_code = typeid(T).hash_code();
        if (type_names.find(hash_code) == type_names.end()) [[unlikely]]
            type_names.emplace(hash_code, get_demangled_name<T>());

        alloc_writer.write(record(scopes.top(), alloc_size, p));
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

        dealloc_writer.write(record(scopes.top(), alloc_size, p));
    }

private:
    profile_state() : scopes(), start_time(get_current_timestamp()) {
        scopes.push(GLOBAL_SCOPE_HASH);
        scope_names.emplace(GLOBAL_SCOPE_HASH, "_global");
        alloc_writer.setup("alloc");
        dealloc_writer.setup("dealloc");
    }

    ~profile_state() {
        alloc_writer.end(start_time, scope_names, type_names);
        dealloc_writer.end(start_time, scope_names, type_names);
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

};

// Exposing the functions to ert:: namespace for ease of use.
template <typename T=default_writer_type>
void push_scope(const std::string& s) {
    profile_state<default_writer_type>::get_state().push_scope(s);
}

template <typename T=default_writer_type>
void pop_scope() {
    profile_state<default_writer_type>::get_state().pop_scope();
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

    template <typename U>
    struct rebind {
        typedef profile_allocator<U,
                                  typename std::allocator_traits<base_allocator>::template rebind_alloc<U>> other;
    };

    /// Default Ctor
    explicit profile_allocator() : alloc(), state(profile_state<default_writer_type>::get_state()) {}

    /// Forward all necessary arguments to the nested allocator.
    /// IMPORTANT: This is a classic example of the perfect forwarding problem, in that if we were to pass in
    /// another profile_allocator and/or base_allocator that's not declared const, this would be picked first.
    template <typename... Args>
    profile_allocator(Args&&... args) noexcept requires (std::is_constructible<base_allocator, Args...>::value) :
            alloc(std::forward<Args>(args)...),
            state(profile_state<default_writer_type>::get_state()) {}

    /// Upon copy construction, we must guarrantee that it must be the base allocator
    /// but we allow it to be any type U templatized by base_allocator. To do this we
    /// unfortunately need to use rebind_alloc.
    /// In addition, to do this, we need to make other profile_allocators friends, since different template instantiations
    /// cannot access each others' private members.
    template <typename U, typename A> friend class profile_allocator;

    // Construct a profile allocator from another profile allocator with the same base_allocator but with different type.
    template <typename U>
    profile_allocator(
        const profile_allocator<U,
                                typename std::allocator_traits<base_allocator>::template rebind_alloc<U>>& other) noexcept :
            alloc(other.alloc), state(profile_state<default_writer_type>::get_state()) {}

    // Construct a profile allocator from its base_allocator with a different type.
    template <typename U>
    profile_allocator(
        const typename std::allocator_traits<base_allocator>::template rebind_alloc<U>& other) noexcept :
            alloc(other), state(profile_state<default_writer_type>::get_state()) {}

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
    profile_state<default_writer_type>& state;
    // We want to call the underlying allocator type
};

template <typename T, typename U, typename base_allocator>
inline bool operator == (const profile_allocator<T, base_allocator>& a,
        const profile_allocator<U, typename std::allocator_traits<base_allocator>::template rebind_alloc<U>>& b)
{
    return a.alloc == b.alloc;
}

template <typename T, typename U, typename base_allocator>
inline bool operator != (const profile_allocator<T, base_allocator>& a,
        const profile_allocator<U, typename std::allocator_traits<base_allocator>::template rebind_alloc<U>>& b)
{
    return a.alloc != b.alloc;
}

} // namespace ert

