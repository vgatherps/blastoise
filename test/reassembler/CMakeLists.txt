include(GoogleTest)
enable_testing()
add_executable(reassembler_test test_most_recent.cc test_forward_all.cc test_fully_ordered.cc)
target_link_libraries(
    reassembler_test
    blastoise
    GTest::gtest_main
)
gtest_discover_tests(reassembler_test)