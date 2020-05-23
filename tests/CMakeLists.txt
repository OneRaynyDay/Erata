add_executable("${PROJECT_NAME}-test" alloc.test.cpp)

target_include_directories("${PROJECT_NAME}-test" PUBLIC ../include)
target_include_directories("${PROJECT_NAME}-test" PUBLIC ../third-party/catch2/single_include)
target_include_directories("${PROJECT_NAME}-test" PUBLIC ../third-party/spdlog/include)

message(STATUS "Building tests...")
