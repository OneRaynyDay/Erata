add_executable("${PROJECT_NAME}-test" alloc_singlethreaded.test.cpp)

target_include_directories("${PROJECT_NAME}-test" PUBLIC ../include)
target_include_directories("${PROJECT_NAME}-test" PUBLIC ../third-party/spdlog/include)

target_link_libraries("${PROJECT_NAME}-test" gtest)

message(STATUS "Building tests...")
