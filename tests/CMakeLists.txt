add_executable("${PROJECT_NAME}-test" alloc_singlethreaded.test.cpp alloc_main.test.cpp)

target_include_directories("${PROJECT_NAME}-test" PUBLIC ../include)
# Required for test writers
target_include_directories("${PROJECT_NAME}-test" PUBLIC include)
target_include_directories("${PROJECT_NAME}-test" PUBLIC ../third-party/spdlog/include)
target_include_directories("${PROJECT_NAME}-test" PUBLIC ../third-party/json/single_include)

target_link_libraries("${PROJECT_NAME}-test" gtest)

message(STATUS "Building tests...")
