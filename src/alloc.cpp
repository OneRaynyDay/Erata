#include "alloc.hpp"
#include <list>

int main() {
    ert::string s("Hello world! Some extra garbage characters so this can be stored on the heap.");
    ert::vector<std::uint32_t> v{1,2,3};
    ert::list<std::uint32_t> l{1,200,3000,400000};

    // PMR examples
    char buffer[1024] {};
    std::pmr::monotonic_buffer_resource rsrc { std::data(buffer), std::size(buffer) };
    ert::pmr::vector<int> pmrv(&rsrc);
    for(int i = 0; i < 10; i++)
        pmrv.push_back(i);

    ert::pmr::list<int> pmrl(&rsrc);
    for(int i = 0; i < 10; i++)
        pmrl.push_back(i);
    return 0;
}