# ── Helper to define a test executable ──────────────────────────
function(phev_add_test name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE phev unity)
    target_include_directories(${name} PRIVATE
        ${unity_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}   # find sibling test_*.c via #include
    )
    add_test(NAME ${name} COMMAND ${name})
endfunction()

# ── Per-suite test executables ─────────────────────────────────
phev_add_test(test_phev_core     run_phev_core.c)
phev_add_test(test_phev_pipe     run_phev_pipe.c)
phev_add_test(test_phev_service  run_phev_service.c)
phev_add_test(test_phev_model    run_phev_model.c)
phev_add_test(test_phev          run_phev.c)
phev_add_test(test_phev_register run_phev_register.c)
