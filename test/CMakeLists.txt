add_executable(test_runner
    test_runner.c
)

target_link_libraries (test_runner LINK_PUBLIC 
    phev
    ${MSG_CORE}
    ${CJSON}
    unity
)
target_include_directories(test_runner INTERFACE ${UNITY})

add_test(test_phev_core test_runner)
