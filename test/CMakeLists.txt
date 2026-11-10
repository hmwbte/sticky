project (sticky_tests)
add_executable(sticky_tests test.cpp)

target_include_directories(sticky_tests PUBLIC "${STICKY_SOURCE_DIR}/include")
target_link_libraries(sticky_tests sticky)
