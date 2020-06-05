#pragma once
#include "alloc.hpp"

namespace ert {
/// forward declaration of alloc_deleter for unique_ptr.
namespace details {
template <typename Allocator>
struct alloc_deleter {
    Allocator a;

    alloc_deleter(const Allocator& a) : a(a) {}
    typedef typename std::allocator_traits<Allocator>::pointer pointer_type;

    void operator()(pointer_type ptr) const {
        Allocator aa(a);
        std::allocator_traits<Allocator>::destroy(aa, std::addressof(*ptr));
        // We allocated a single element of this.
        std::allocator_traits<Allocator>::deallocate(aa, ptr, 1);
    }
};
} // namespace details

/// ert's shared_ptr is the same as the std shared_ptr thanks to the fact that
/// stl already type-erases the deleter of the shared_ptr.
template <typename T>
using shared_ptr = std::shared_ptr<T>;

/// TODO: Issue - unique_ptr is no longer zero copy.
/// Why? because we need to pass in a deleter that is allocator aware. An allocator-aware
/// deleter must know the type of the allocator and if it's general, must be able to take an
/// allocator and copy construct it. This can't cost 0 bytes per unique_ptr.
template <typename T>
using default_delete = details::alloc_deleter<profile_allocator<T, std::allocator<T>>>;
template <typename T, typename Deleter=default_delete<T>>
using unique_ptr = std::unique_ptr<T, Deleter>;

/// Allocate a shared_ptr type using a wrapped version of allocator
template <typename T, typename Alloc, typename... Args>
std::shared_ptr<T> allocate_shared(const Alloc& alloc, Args&&... args) {
    auto prof_alloc = profile_allocator<T, Alloc>(alloc);
    return std::allocate_shared<T>(prof_alloc, std::forward<Args>(args)...);
}

/// Create a shared_ptr from a default stl allocator wrapped in profile allocator.
template <typename T, typename... Args>
std::shared_ptr<T> make_shared(Args&&... args) {
    return ert::allocate_shared<T>(std::allocator<T>(), std::forward<Args>(args)...);
}

/// Allocate a unique_ptr type using a wrapped version of allocator.
/// NOTE: Because the STL does not have alloc_unique yet, we will supply an implementation ourselves.
template <typename T, typename Alloc, typename... Args>
auto allocate_unique(const Alloc& alloc, Args&&... args) {
    // Allow rebinding of the allocator type
    using rebound_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
    auto prof_alloc = profile_allocator<T, rebound_allocator_type>(alloc);

    using prof_alloc_traits = typename std::allocator_traits<decltype(prof_alloc)>;

    // Create exactly 1 of such object
    auto ptr = prof_alloc_traits::allocate(prof_alloc, 1);
    try {
        prof_alloc_traits::construct(prof_alloc, std::addressof(*ptr), std::forward<Args>(args)...);
        using deleter = details::alloc_deleter<decltype(prof_alloc)>;
        return ert::unique_ptr<T, deleter>(ptr, deleter(prof_alloc));
    }
    catch (...) {
        prof_alloc_traits::deallocate(prof_alloc, ptr, 1);
        throw;
    }
}

/// Create a unique_ptr from a default stl allocator wrapped in profile allocator.
template <typename T, typename... Args>
ert::unique_ptr<T> make_unique(Args&&... args) {
    return ert::allocate_unique<T>(std::allocator<T>(), std::forward<Args>(args)...);
}

} // namespace ert