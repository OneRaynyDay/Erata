# Erata

Measuring heap fragmentation by providing a modern, header-only C++20 metrics-collecting allocator and corresponding allocator-aware containers.

**NOTE**: The library isn't fully implemented yet. So far, only single-threaded implementation is supported, and g++ is known to be supported.

## Installation

To use the allocator, simply copy and include the [header file](https://raw.githubusercontent.com/OneRaynyDay/Erata/master/include/alloc.hpp) `alloc.hpp` in this repo.

## Visualization

To generate the sample visualization, run `python3 client/generate.py`. This makes a copy of `client/template.html`, and prepends a `<script />` tag with data parsed from `client/test_data.json`. Then, you can just open `client/index.html` in your browser, and it will console log sample data.

## How to Use

```c++
#include "alloc.hpp"
#include "stl_containers.hpp"

using namespace ert;

int main() {
    ert::push_scope("stl");
    {
        ert::string s("Hello world! Some extra garbage characters so this can be stored on the heap.");
        ert::push_scope("vector");
        ert::vector<std::uint32_t> v{1, 2, 3};
        ert::pop_scope();
        ert::push_scope("list");
        ert::list<std::uint32_t> l{1, 200, 3000, 400000};
        ert::pop_scope();
    }
    ert::pop_scope();

    ert::push_scope("pmr");
    // PMR examples
    {
        char buffer[1024]{};
        std::pmr::monotonic_buffer_resource rsrc{std::data(buffer), std::size(buffer)};
        ert::pmr::vector<int> pmrv(&rsrc);
        for (int i = 0; i < 10; i++)
            pmrv.push_back(i);

        ert::pmr::list<int> pmrl(&rsrc);
        for (int i = 0; i < 10; i++)
            pmrl.push_back(i);
    }
    ert::pop_scope();

    ert::push_scope("shared_ptr<T>");
    // shared_ptr examples
    {
        ert::shared_ptr<ert::vector<int>> ptr = ert::make_shared<ert::vector<int>>();
        ert::push_scope("vector");
        ptr->push_back(1);
        ptr->push_back(1);
        ptr->push_back(1);
        ptr->push_back(1);
        ert::pop_scope();
    }
    // Currently, this prints out 16, 16.
    fmt::print("size of shared_ptrs : stl: {}, ert: {}", sizeof(std::shared_ptr<int>), sizeof(ert::shared_ptr<int>));

    ert::push_scope("unique_ptr<T>");
    // shared_ptr examples
    {
        ert::unique_ptr<ert::vector<int>> ptr = ert::make_unique<ert::vector<int>>();
        ert::push_scope("vector");
        ptr->push_back(1);
        ptr->push_back(1);
        ptr->push_back(1);
        ptr->push_back(1);
        ert::pop_scope();
    }
    // Currently, this prints out 8, 24. Optimizing this is on our TODO list.
    fmt::print("size of unique_ptrs : stl: {}, ert: {}", sizeof(std::unique_ptr<int>), sizeof(ert::unique_ptr<int>));

    // It's thread-safe!
    auto thread_alloc = [](){
        ert::list<int> v;
        v.push_back(1);
        v.push_back(1);
        v.push_back(1);
        v.push_back(1);
        v.push_back(1);
    };

    ert::vector<std::thread> threads;
    for (int i = 0; i < 10; i++) {
        threads.emplace_back(thread_alloc);
    }

    for (int i = 0; i < 10; i++) {
        threads[i].join();
    }

    return 0;
}
```

and it will generate a `.json` file in the executable's folder containing allocation information. The data visualization portion is still WIP, and reading the json file is difficult right now
(since the data serialized is optimized for performance).

## Limitations (for now)

We want to make this as streamlined as possible for any users attempting to switch between test and prod environments, but there are some inherent limitations that we have to deal with.

These limitations are listed below:

- The user must use the ert namespace substitutes for stl and smart pointer allocations. Polluting std namespace yields undefined behavior.
- If we were to override the default allocator, this is only possible right now in `g++` and not `clang`, but users will not have to change namespaces. I will attempt this but it's not cross-compiler supported.
- `ert::unique_ptr`s can no longer be zero-cost because we need to set default deleter to use allocators, specifically `ert::profile_allocator<T, std::allocator<T>>`. So **production memory usage may be slightly different than test usage**.
