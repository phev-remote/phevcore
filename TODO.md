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

- [x] Choose test framework: **greatest** (v1.5.0)
- [x] Add framework via FetchContent
- [x] Convert `test_phev_core.c` (82 tests — 66 pass, 16 fail from previously-unwired tests)
- [x] Convert `test_phev_pipe.c` (31 tests — 21 pass, 10 fail from previously-unwired tests)
- [x] Convert `test_phev_service.c` (70 tests — 60 pass, 9 fail, 1 skip from previously-unwired tests)
- [x] Convert `test_phev_model.c` (8 tests — all pass)
- [x] Convert `test_phev.c` (2 tests — all pass)
- [x] Convert `test_phev_register.c` (14 tests — all pass)
- [x] Wire all previously-unwired test functions (35 fail + 1 skip are pre-existing bugs, not regressions)
- [x] Triage orphaned files:
  - `test_phev_config.c` — converted to greatest (12 tests, all pass), fixture `test/config.json` created
  - `test_phev_controller.c` — deleted (requires CMock + missing source `phev_controller.c`)
  - `test_phev_response_handler.c` — deleted (empty, 0 tests)
- [x] Delete all `run_*.c` Unity shims
- [x] Delete `test_runner.c` and Unity FetchContent (Unity was already removed)
- [x] Verify all 219 tests run across 7 suites — 183 pass, 35 fail, 1 skip (zero regressions)

### Bug fixes applied during migration
- Fixed `include/logger.h` `hexdump()` stack-buffer-underflow (line 89)
- Added missing `phev_core_validateChecksumXOR` declaration to `include/phev_core.h`
- Skipped `test_phev_service_jsonInputTransformer` — pre-existing segfault from NULL pipe context

## Phase 3 — Directory restructure

- [x] Move `src/` into `src/msg/` + `src/phev/`
- [x] Move `include/` into `include/msg/` + `include/phev/`
- [x] Update all `#include` directives (79 directives across 24 files)
- [x] Split CMake into two targets: `msg_core` (static) + `phev` (static, links `msg_core`)
- [x] Gate dead transport backends (`msg_gcp_mqtt`, `msg_mqtt_paho`) behind `BUILD_TRANSPORT_BACKENDS` option
- [x] Update install rules for new directory layout
- [x] Tests remain in `test/` (no msg-layer tests exist; all 7 suites test phev)
- [x] Dockerfile and CI already use presets — no changes needed
- [x] Update `AGENTS.md` to reflect restructure
