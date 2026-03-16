# AGENTS.md

## Purpose
- This repository is a small C11 library for talking to Mitsubishi Outlander PHEV remote WiFi systems.
- Core code lives in `src/`, public headers in `include/`, and Unity tests in `test/`.
- The build is CMake-based; `cJSON` and Unity are fetched via FetchContent. `msg-core` sources are vendored directly in `src/`.

## Repository Layout
- `src/`: library implementation files — both phev-specific (`phev.c`, `phev_core.c`, `phev_service.c`, `phev_pipe.c`, etc.) and vendored msg-core (`msg_core.c`, `msg_pipe.c`, `msg_utils.c`, etc.).
- `include/`: installed public headers such as `phev.h`, `phev_core.h`, `phev_service.h`, `msg_core.h`, `msg_pipe.h`.
- `test/`: Unity-based test sources, per-suite `run_*.c` runners, and `test/CMakeLists.txt`.
- `CMakeLists.txt`: root build definition for the static library and optional tests.
- `CMakePresets.json`: standardized build presets (dev, release, ci).
- `Dockerfile`: reproducible build that uses the `ci` preset and runs `ctest`.
- `.github/workflows/dockerimage.yml`: GitHub Actions CI that builds and tests natively with cmake presets.
- `.clang-format`: documents the project's formatting conventions (not enforced).
- `TODO.md`: tracks the multi-phase restructure plan.

## Dependencies
- Build/runtime: `cJSON` (fetched via FetchContent, v1.7.18).
- Test: greatest (fetched via FetchContent, v1.5.0).
- msg-core sources are vendored directly in `src/` and `include/` (originally from github.com/papawattu/msg-core).

## Build Commands

### Using presets (recommended, requires CMake >= 3.21)
```sh
# Development (debug + tests + compile_commands.json)
cmake --preset dev && cmake --build --preset dev

# Release (optimized, no tests)
cmake --preset release && cmake --build --preset release

# CI (release + tests)
cmake --preset ci && cmake --build --preset ci
```

### Manual configuration
```sh
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build
```

### Install artifacts
```sh
cmake --install build
```

## Test Commands
- Run all tests via preset:
```sh
ctest --preset dev
```
- Run all tests manually:
```sh
ctest --test-dir build --output-on-failure
```
- Run a specific test suite:
```sh
ctest --test-dir build -R '^test_phev_core$' --output-on-failure
```
- Run a test executable directly:
```sh
./build/test/test_phev_core
```
- Run the Dockerized test flow:
```sh
docker build -t phevcore . && docker run --rm phevcore
```

## Test Suites
CTest registers 6 per-suite executables, each with its own `run_*.c` runner:
- `test_phev_core` (52 tests)
- `test_phev_pipe` (18 tests)
- `test_phev_service` (56 tests)
- `test_phev_model` (8 tests)
- `test_phev` (2 tests)
- `test_phev_register` (14 tests)

A legacy monolithic `test_runner.c` also exists but is not wired into the CMake build.

## Single-Test Guidance
- There is no built-in per-test-name CLI filter in the current Unity runners.
- If you need one specific Unity case, the least invasive approach is to temporarily comment out unrelated `RUN_TEST(...)` lines in the relevant `test/run_*.c` file, build, run, then restore.
- If you need repeatable focused execution, add a dedicated temporary runner in `test/` rather than reshaping production code.

## Lint / Static Analysis
- A `.clang-format` file documents the project's formatting conventions (4-space indent, Allman braces, middle pointer alignment).
- It is not enforced automatically; use it with editor integrations or manual `clang-format` runs when desired.
- There is no other repository-defined lint or static analysis command.

## CI / Verification
- GitHub Actions runs on push/PR to `main`: configure, build, and test using the `ci` preset.
- The Dockerfile also uses the `ci` preset and can be used for local verification:
```sh
docker build -t phevcore . && docker run --rm phevcore
```
- For quick local verification after code changes:
```sh
cmake --preset dev && cmake --build --preset dev && ctest --preset dev
```

## Language and Build Conventions
- Target language is C11: `set(CMAKE_C_STANDARD 11)`.
- The main artifact is a static library named `phev`.
- Tests are only added when `BUILD_TESTS` is enabled.
- Public headers are intended for installation from `include/`.

## Import / Include Style
- Put standard library headers first, then project headers, then external library headers if needed.
- Use quoted includes for project headers, for example `#include "phev_core.h"`.
- Use angle brackets for standard headers such as `<stdlib.h>` and `<stdint.h>`.
- Keep include blocks compact; do not alphabetize aggressively if existing local grouping is clearer.
- Many headers define `_GNU_SOURCE` guards at the top; preserve that pattern where GNU extensions are required.

## Formatting Style
- Follow the existing 4-space indentation.
- Opening braces usually go on the next line for functions and control statements (Allman style).
- Keep one statement per line.
- Use spaces inside control keywords: `if (...)`, `switch (...)`, `for (...)`.
- Multi-line struct initializers commonly use one field per line with leading `.` designators.
- Keep line wrapping pragmatic; this codebase does not enforce a strict column limit.
- See `.clang-format` for the machine-readable style definition.

## Naming Conventions
- Public and private functions use the `phev_` prefix (or `msg_` for msg-core).
- Types use `_t` suffixes, for example `phevCtx_t`, `phevMessage_t`, `phevServiceCtx_t`.
- Constants and protocol/register macros use upper snake case, for example `KO_WF_H_LAMP_CONT_SP`.
- Local log tags are usually `const static char *TAG` or `APP_TAG`.
- Test functions use `test_...` naming and are invoked explicitly with `RUN_TEST(...)`.

## Types and Data Handling
- Use fixed-width integer types from `<stdint.h>` for protocol data.
- Use `bool` from `<stdbool.h>` for binary state.
- Prefer `size_t` for lengths and allocation sizes.
- Protocol payloads are byte arrays (`uint8_t *`) and often represented as flexible array members.
- Preserve existing signed/unsigned behavior carefully; message bytes and register values are treated as raw bytes.

## Error Handling
- Existing code usually handles errors by returning `NULL`, `false`, `0`, or `-1` depending on API shape.
- Validate pointer arguments early when adding new code.
- Log failures with `LOG_E` or suspicious situations with `LOG_W`.
- Maintain current callback-based error propagation where present, such as `phevErrorHandler_t`.
- Do not introduce exceptions-style abstractions; stay idiomatic C.

## Memory Management
- Most domain objects are heap-allocated manually with `malloc` and released with `free` or domain-specific destroy helpers.
- Be explicit about ownership when returning allocated memory.
- Important examples of heap-returning APIs: JSON strings, copied registers, decoded messages, HVAC status structs.
- When extending APIs, document whether the caller or callee frees returned memory.
- Avoid hidden ownership transfer unless there is already an established helper such as `phev_core_destroyMessage()` or `msg_utils_destroyMsg()`.
- In the msg-core framework, messages returned by a responder are freed by the framework after being published. The splitter loop also frees each split message after processing through the transform chain. Do not double-free messages that the framework owns.

## Logging and Diagnostics
- Logging is pervasive and uses macros like `LOG_V`, `LOG_D`, `LOG_I`, `LOG_W`, `LOG_E`.
- Many functions log `START` and `END`; keep that style in touched code if the surrounding file already uses it.
- Prefer repository logging macros over raw `printf`, except in existing buffer-dump helpers or tests.

## Testing Conventions
- Tests are integration-heavy and often exercise real message encoding/decoding paths.
- Each test suite has a dedicated `test/run_*.c` runner that `#include`s the corresponding `test_*.c` file and wires `RUN_TEST(...)` entries.
- Add new test functions to the relevant `test/test_*.c` file and wire them into `RUN_TEST(...)` in the matching `test/run_*.c` runner.
- Follow existing assertion style with Unity macros such as `TEST_ASSERT_EQUAL`, `TEST_ASSERT_NOT_NULL`, and `TEST_ASSERT_EQUAL_HEX8_ARRAY`.

## Editing Guidance For Agents
- Keep changes narrow and consistent with surrounding style; this is not a heavily normalized codebase.
- Prefer fixing bugs in-place over broad stylistic cleanup.
- Do not silently rename public macros, structs, or functions without updating all call sites and tests.
- Be careful around protocol constants and XOR/checksum logic in `phev_core.c`; small byte-level changes can invalidate many tests.
- Be careful around callback registration and pipe/service context wiring; several modules pass context through nested structs.

## Known Quirks To Respect
- Some code intentionally uses duplicated patterns, manual memory management, and verbose logging; preserve behavior first, elegance second.
- There are existing rough edges and probable bugs in the codebase; avoid opportunistic rewrites unless required for the task at hand.
- 75 of 220 defined test functions are not wired into any runner (34%). See `TODO.md` for the Phase 2 plan to address this.
- `test_phev_config.c` (12 tests) and `test_phev_controller.c` (5 active tests, needs CMock) are orphaned with no runner.
