#include "alloc.hpp"
#include <list>

int main() {
    ert::string s("Hello world! Some extra garbage characters so this can be stored on the heap.");
    ert::vector<std::uint32_t> v{1,2,3};
    ert::list<std::uint32_t> l{1,200,3000,400000};
    return 0;
}