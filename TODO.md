# Restructure TODO

Tracks the multi-phase restructure of the phevcore build system and
project layout. Each phase is one PR.

## Phase 1 — Build modernization (this PR)

- [x] Add `CMakePresets.json` (dev / release / ci)
- [x] Remove `splint.cmake` stub
- [x] Fix GitHub Actions CI to run tests natively
- [x] Update Dockerfile to use presets
- [x] Remove dead files (`.travis.yml`, `runtests.bat`, `.gitmodules`, CSV)
- [x] Expand `.gitignore`
- [x] Add `.clang-format`
- [x] Update `AGENTS.md` to reflect changes

## Phase 2 — Test migration

- [ ] Choose test framework (greatest or CMocka)
- [ ] Add framework via FetchContent
- [ ] Convert `test_phev_core.c` (83 tests, 31 never wired)
- [ ] Convert `test_phev_pipe.c` (33 tests, 15 never wired)
- [ ] Convert `test_phev_service.c` (59 tests, 14 never wired)
- [ ] Convert `test_phev_model.c` (8 tests, fully wired)
- [ ] Convert `test_phev.c` (4 tests, 2 never wired)
- [ ] Convert `test_phev_register.c` (16 tests, 2 never wired)
- [ ] Wire all 75 currently-unwired test functions
- [ ] Triage orphaned files: `test_phev_config.c` (12 tests, needs fixture path fix), `test_phev_controller.c` (CMock dependency), `test_phev_response_handler.c` (empty)
- [ ] Delete `run_*.c` shims, `test_runner.c`, and Unity FetchContent
- [ ] Verify all tests pass, confirm per-test CLI filtering works

## Phase 3 — Directory restructure

- [ ] Move `src/` into `src/msg/` + `src/phev/`
- [ ] Move `include/` into `include/msg/` + `include/phev/`
- [ ] Update all `#include` directives
- [ ] Split CMake into two targets: `msg_core` (static) + `phev` (static, links `msg_core`)
- [ ] Move tests into `tests/msg/` + `tests/phev/`
- [ ] Gate dead transport backends (`msg_gcp_mqtt`, `msg_mqtt_paho`) behind CMake options
- [ ] Update Dockerfile, install rules, and CI
