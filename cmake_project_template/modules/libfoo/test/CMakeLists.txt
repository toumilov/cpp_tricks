find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_executable(foo_test
	foo_test.cpp
)
set_target_properties(foo_test PROPERTIES OUTPUT_NAME foo.test)
target_link_libraries(foo_test PRIVATE libfoo-static GTest::gtest)
add_custom_target(unittests_foo COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS foo_test)

gtest_discover_tests(foo_test)