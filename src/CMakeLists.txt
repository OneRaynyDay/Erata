add_executable("${PROJECT_NAME}-driver" alloc.cpp)

target_include_directories("${PROJECT_NAME}-driver" PUBLIC ../include)
target_include_directories("${PROJECT_NAME}-driver" PUBLIC ../third-party/spdlog/include)
target_include_directories("${PROJECT_NAME}-driver" PUBLIC ../third-party/json/single_include)

message(STATUS "Building driver...")
