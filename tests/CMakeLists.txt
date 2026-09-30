add_executable(twf_tests main_tests.cpp)
target_link_libraries(twf_tests PRIVATE twf)
add_test(NAME twf_unit_tests COMMAND twf_tests)
