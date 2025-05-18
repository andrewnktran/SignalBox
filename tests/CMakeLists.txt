enable_testing()
add_executable(test_parser test_parser.cpp)
target_link_libraries(test_parser gtest gtest_main)
add_test(NAME TestParser COMMAND test_parser)
