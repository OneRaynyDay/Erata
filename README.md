# Erata
Measuring heap fragmentation by providing a modern, header-only C++17 metrics-collecting allocator and corresponding allocator-aware containers.

**NOTE**: The library isn't fully implemented yet. Not even close. Pls chill

## Installation

To use the allocator, simply copy and include the [header file](https://raw.githubusercontent.com/OneRaynyDay/Erata/master/include/alloc.hpp) `alloc.hpp` in this repo.


## How to Use

```c++
#include "alloc.hpp"

using namespace ert;

int main() {

    ert::set_scope('main');
    std::shared_ptr<int> x = ert::make_shared<int>(2); 
    {
        ert::set_scope('stl containers');
        ert::vector<int> v(100);
    }
    std::unique_ptr<std::string> x = ert::make_unique<std::string>("Hello world!");
}
```

We want to make this as streamlined as possible for any users attempting to switch between test and prod environments, but there are some inherent limitations that we have to deal with.

These limitations are listed below:

- The user must use the ert namespace substitutes for stl and smart pointer allocations. Polluting std namespace yields undefined behavior.
- If we were to override the default allocator, this is only possible right now in `g++` and not `clang`, but users will not have to change namespaces. I will attempt this but it's not cross-compiler supported.

