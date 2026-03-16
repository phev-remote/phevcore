add_executable(test_runner
    test_runner.c
)

target_link_libraries(test_runner PRIVATE
    phev
    unity
)

# Unity headers from FetchContent
target_include_directories(test_runner PRIVATE
    ${unity_SOURCE_DIR}/src
)

add_test(NAME test_phev_core COMMAND test_runner)
