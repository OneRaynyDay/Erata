#pragma once

#define NO_STL

// For memcpy
#include <cstring>

// For runtime error
#include <stdexcept>

#include <string>
#include <unordered_map>

namespace ert {

namespace details {

// Why the hell are we writing our own stl containers?
// Because we have no choice if we want to set the allocator as default in gcc.
// These aren't in the hotpath, so we don't worry about performance too much.

template <typename T>
class vector {
    using ptr_type = T*;
    // Should be enough for most cases, but expands as needed.
    static constexpr std::size_t initial_capacity = 8;
    // Multiplies by 2 in size everytime.
    static constexpr std::size_t growth_ratio = 2;

public:
    vector() noexcept : _members(new T[initial_capacity]) , _size(0), _capacity(initial_capacity) {}
    vector(const vector& s) noexcept {
        _size = s._size;
        _capacity = s._capacity;
        _members = new T[_capacity];

        // No need to copy everything, just up to size is fine.
        std::memcpy(_members, s._members, sizeof(T) * _size);
    }
    vector(vector&& s) : _size(s._size), _capacity(s._capacity), _members(s._members){
        // Invalidates the members in the previously owning vector.
        s._members = nullptr;
        s._size = 0;
    }
    vector& operator=(vector s) noexcept {
        // The members pointer in this class gets assigned to a temporary and is freed.
        vector tmp(s);
        swap(tmp);
        return *this;
    }
    vector& operator=(vector&& s) {
        // Invalidate current
        delete [] _members;
        _members = nullptr;
        _size = 0;

        // Swap with the other - it becomes lvalue here if I don't std::move, and we can just work w/ it:
        // https://vectoroverflow.com/questions/54555189/swap-functions-for-lvalue-and-rvalue-references
        swap(s);
        return *this;
    }

    // User-facing API
    void swap(vector& s) {
        std::swap(_members, s._members);
        std::swap(_size, s._size);
        std::swap(_capacity, s._capacity);
    }

    void push_back(const T& t) {
        // Need to reserve new capacity.
        if (_size >= _capacity) [[unlikely]] {
            ptr_type old_members = _members;
            std::size_t old_capacity = _capacity;

            _capacity *= growth_ratio;
            _members = new T[_capacity];

            // copy the contents
            std::copy(old_members, old_members + old_capacity, _members);

            // Reclaim old resources
            delete [] old_members;
        }
        _members[_size++] = t;
    }

    [[nodiscard]] std::size_t size() const noexcept { return _size; }

    T& operator[](std::size_t n) {
        return _members[n];
    }

    ~vector() noexcept {
        delete [] _members;
    }

private:
    std::size_t _size;
    std::size_t _capacity;
    ptr_type _members;
};

template <typename T>
class stack {
    using ptr_type = T*;
    // Should be enough for most cases, but expands as needed.
    static constexpr std::size_t initial_capacity = 8;
    // Multiplies by 2 in size everytime.
    static constexpr std::size_t growth_ratio = 2;
public:
    // ctors/dtors
    stack() noexcept : _members(new T[initial_capacity]) , _size(0), _capacity(initial_capacity) {}
    stack(const stack& s) noexcept {
        _size = s._size;
        _capacity = s._capacity;
        _members = new T[_capacity];

        // No need to copy everything, just up to size is fine.
        std::memcpy(_members, s._members, sizeof(T) * _size);
    }
    stack(stack&& s) : _size(s._size), _capacity(s._capacity), _members(s._members){
        // Invalidates the members in the previously owning stack.
        s._members = nullptr;
        s._size = 0;
    }
    stack& operator=(stack s) noexcept {
        // The members pointer in this class gets assigned to a temporary and is freed.
        stack tmp(s);
        swap(tmp);
        return *this;
    }
    stack& operator=(stack&& s) {
        // Invalidate current
        delete [] _members;
        _members = nullptr;
        _size = 0;

        // Swap with the other - it becomes lvalue here if I don't std::move, and we can just work w/ it:
        // https://stackoverflow.com/questions/54555189/swap-functions-for-lvalue-and-rvalue-references
        swap(s);
        return *this;
    }
    ~stack() noexcept {
        delete [] _members;
    }

    // User-facing API
    void swap(stack& s) {
        std::swap(_members, s._members);
        std::swap(_size, s._size);
        std::swap(_capacity, s._capacity);
    }

    void push(const T& t) {
        // Need to reserve new capacity.
        if (_size >= _capacity) [[unlikely]] {
            ptr_type old_members = _members;
            std::size_t old_capacity = _capacity;

            _capacity *= growth_ratio;
            _members = new T[_capacity];

            // copy the contents
            std::copy(old_members, old_members + old_capacity, _members);

            // Reclaim old resources
            delete [] old_members;
        }
        _members[_size++] = t;
    }

    void pop() {
        if (_size == 0) [[unlikely]]
            throw std::runtime_error("Stack has 0 size and cannot pop. This should never happen from application code.");
        _size--;
    }

    [[nodiscard]] T top() const {
        if (_size == 0) [[unlikely]]
            throw std::runtime_error("Stack has 0 size, nothing on top. This should never happen from application code.");
        return _members[_size - 1];
    }

    [[nodiscard]] std::size_t size() const noexcept { return _size; }
private:
    std::size_t _size;
    std::size_t _capacity;
    ptr_type _members;
};

// Even though the assumption that # of types is relatively small for C++ programs is
// reasonable for most applications, we don't want to
template <typename T>
class map {
    // Once the map exceeds 0.75 the capacity, we do a resize.
    static constexpr float load_factor = 0.75;
    static constexpr std::size_t initial_capacity = 128;
    using key_type = std::size_t;
    using pair_type = std::pair<key_type, T>;
    using container_type = details::vector<pair_type>;
    using container_ptr = container_type*;
public:
    map() noexcept : _size(0), _num_buckets(initial_capacity), _buckets(new container_type[initial_capacity]) {}
    map(const map& m) {
        _size = m._size;
        _num_buckets = m._num_buckets;
        _buckets = new container_type[_num_buckets];

        // We need to perform a deep copy, which means we need to invoke the copy constructor of each container
        // Each bucket needs to be copied.
        std::copy(m._buckets, m._buckets + _num_buckets, _buckets);
    }

    // Returns the pointer to location of k if it exists. If not, then return nullptr
    pair_type* find(const key_type& k) const {
        auto bucket = _buckets[k % _num_buckets];
        // TODO: optimize
        // performs a linear search in bucket for existing key.
        for (int i = 0; i < bucket.size(); i++) {
            auto& p = bucket[i];
            // Found existing pair and this just performs an update.
            if (p.first == k) {
                return &p;
            }
        }
        return nullptr;
    }

    constexpr pair_type* end() const noexcept { return nullptr; }

    void emplace(const key_type& k, const T& t) {
        auto bucket = _buckets[k % _num_buckets];
        // TODO: optimize
        // performs a linear search in bucket for existing key.
        for (int i = 0; i < bucket.size(); i++) {
            auto& p = bucket[i];
            // Found existing pair and this just performs an update.
            if (p.first == k) {
                p.second = t;
                return;
            }
        }
        // Did not find existin gpair and this will add to the bucket.
        bucket.push( {k, t} );
    }


private:
    std::size_t _size;
    std::size_t _num_buckets;
    container_ptr _buckets;
};

} // namespace details

/// We need timestamp for both record and the beginning of the program.
using timestamp_type = unsigned long;
using location_type = unsigned long;

/// IMPORTANT: We don't even expose an API to the user about searching for such maps.
/// It is sufficient to specialize for <int, T> case of maps.
/// The structures can be append-only.
#ifdef NO_STL
using scope_map = details::map<std::string>;
using type_map = details::map<std::string>;
using scope_stack = details::stack<std::size_t>;
#else
using scope_map = std::unordered_map<std::size_t, std::string>;
using type_map = std::unordered_map<std::size_t, std::string>;
using scope_stack = details::stack<std::size_t>;
#endif

} // namespace ert
