# Restructure TODO

Tracks the multi-phase restructure of the phevcore build system and
project layout. Each phase is one PR.

## Phase 1 — Build modernization ✓

- [x] Add `CMakePresets.json` (dev / release / ci)
- [x] Remove `splint.cmake` stub
- [x] Fix GitHub Actions CI to run tests natively
- [x] Update Dockerfile to use presets
- [x] Remove dead files (`.travis.yml`, `runtests.bat`, `.gitmodules`, CSV)
- [x] Expand `.gitignore`
- [x] Add `.clang-format`
- [x] Update `AGENTS.md` to reflect changes

## Phase 2 — Test migration ✓

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

## Phase 3 — Directory restructure ✓

- [x] Move `src/` into `src/msg/` + `src/phev/`
- [x] Move `include/` into `include/msg/` + `include/phev/`
- [x] Update all `#include` directives (79 directives across 24 files)
- [x] Split CMake into two targets: `msg_core` (static) + `phev` (static, links `msg_core`)
- [x] Gate dead transport backends (`msg_gcp_mqtt`, `msg_mqtt_paho`) behind `BUILD_TRANSPORT_BACKENDS` option
- [x] Update install rules for new directory layout
- [x] Tests remain in `test/` (no msg-layer tests exist; all 7 suites test phev)
- [x] Dockerfile and CI already use presets — no changes needed
- [x] Update `AGENTS.md` to reflect restructure

## Phase 4 — Fix SKIPped tests (branch: `phase4/fix-skipped-tests`) ✓

Goal: fix all 36 tests that were SKIPped (pre-existing bugs from previously-unwired
 test functions) and reduce the skip count to zero.

### Source fixes applied
- [x] `phev_core_getType()` — XOR-decode the type byte before classifying (`src/phev/phev_core.c`)
- [x] `phev_core_encodeMessage()` — apply XOR to encoded bytes (`src/phev/phev_core.c`)
- [x] `phev_service_jsonInputTransformer()` — guard against NULL `ctx->pipe` (`src/phev/phev_service.c`)
- [x] `phev_pipe_outputChainInputTransformer()` — use decoded `phevMessage->XOR` instead of `message->ctx` for XOR detection; return decoded message when XOR is active (`src/phev/phev_pipe.c`)
- [x] `phev_pipe_commandResponder()` — propagate XOR from `message->ctx` to `phevMsg.XOR` so response is correctly encoded (`src/phev/phev_pipe.c`)
- [x] `phev_core_xorDataOutbound()` / `phev_core_XOROutboundMessage()` — use `message->length` instead of `data[1]+2` to handle concatenated messages (`src/phev/phev_core.c`)
- [x] `phev_service_validateCommand()` — add missing "update" operation validation (`src/phev/phev_service.c`)

### test_phev_core.c — 16 SKIPs → 0 (all 82 pass)
- [x] Group 1 (4 tests): fix checksums and expected data for XOR-encoded messages
- [x] Group 2 (5 tests): fix expected XOR values and command/ack expectations
- [x] Group 3 (4 tests): fix expected ping constant (0xf6 → 0xf9)
- [x] Group 4 (3 tests): fix expected data arrays for XOR encoding

### test_phev_service.c — 10 SKIPs → 0 (all 70 pass)
- [x] Group 6 (5 tests): update expected data and checksum for XOR encoding
- [x] Group 7 (4 tests): fix expected bytes for service-layer encoding
- [x] Group 10 (1 test, line 263): `validateCommand()` missing "update" operation

### test_phev_pipe.c — 10 SKIPs → 0 (all 31 pass)
- [x] Group 5 (3 tests): `commandXOR` propagation through pipe context
- [x] Group 6 (2 tests): `outputChainInputTransformer` with XOR
- [x] Group 7 (1 test + 1 bonus): XOR handling with NULL pipe context / ping response
- [x] Group 8 (2 tests): `sendMac` / `start_my18` wiring — fix `xorDataOutbound` length
- [x] Group 9 (1 test): `waitForConnection` retry loop — add failing connect stub

### Summary
- Started: 36 SKIPped tests (16 + 10 + 10)
- Fixed: 36 (all)
- Remaining: 0
- All 219 tests pass across 7 suites, 0 fail, 0 skip

## Phase 5 — Cleanup ✓

- [x] Remove dead transport source files (`msg_gcp_mqtt.*`, `msg_mqtt_paho.*`)
- [x] Remove stale `config.h` (unused)
- [x] Remove `BUILD_TRANSPORT_BACKENDS` option and install block from `CMakeLists.txt`
- [x] Clean up stale remote branches (deleted 6: phase1-3, add-license-1, register_fix, robustxor)
- [x] Clean up stale local branches (deleted phase1-3)
- [x] Update `AGENTS.md` to reflect all-tests-passing and removal of transport backends
- [x] Final CI verification — dev preset build + all 219 tests pass, release preset builds clean
