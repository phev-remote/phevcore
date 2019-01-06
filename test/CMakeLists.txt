add_executable(test_runner
    test_runner.c
)

target_link_libraries(test_runner
    msg_core
    phev_core
    Unity
    cjson
)

add_test(test_phev_core test_runner)

target_include_directories(msg_core PUBLIC ../external/msg_core/include)
