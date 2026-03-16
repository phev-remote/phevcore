# ── Helper to define a test executable ──────────────────────────
function(phev_add_test name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE phev)
    target_include_directories(${name} PRIVATE
        ${greatest_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}   # find sibling test_*.c via #include
    )
    add_test(NAME ${name} COMMAND ${name})
endfunction()

# ── Per-suite test executables (all use greatest) ─────────────
phev_add_test(test_phev_core     test_phev_core.c)
phev_add_test(test_phev_pipe     test_phev_pipe.c)
phev_add_test(test_phev_service  test_phev_service.c)
phev_add_test(test_phev_model    test_phev_model.c)
phev_add_test(test_phev          test_phev.c)
phev_add_test(test_phev_register test_phev_register.c)

# test_phev_config needs the config.json fixture path
phev_add_test(test_phev_config   test_phev_config.c)
target_compile_definitions(test_phev_config PRIVATE
    CONFIG_JSON_PATH="${CMAKE_CURRENT_SOURCE_DIR}/config.json"
)
