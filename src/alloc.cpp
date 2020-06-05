#include "alloc.hpp"
#include "pointer_types.hpp"
#include "stl_containers.hpp"
#include <thread>
#include <list>
#include <vector>

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
        ert::pmr::monotonic_buffer_resource rsrc{std::data(buffer), std::size(buffer)};
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
    ert::pop_scope();
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
    ert::pop_scope();
    // Currently, this prints out 8, 24. Optimizing this is on our TODO list.
    fmt::print("size of unique_ptrs : stl: {}, ert: {}", sizeof(std::unique_ptr<int>), sizeof(ert::unique_ptr<int>));

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