# AGENTS.md

## Purpose
- This repository is a small C11 library for talking to Mitsubishi Outlander PHEV remote WiFi systems.
- Core code lives in `src/`, public headers in `include/`, and Unity tests in `test/`.
- The build is CMake-based and depends on external `msg-core`, `cJSON`, and optionally Unity for tests.
- `splint.cmake` is a stub file to satisfy the CMake include.

## Repository Layout
- `src/`: library implementation files such as `phev.c`, `phev_core.c`, `phev_service.c`, `phev_pipe.c`.
- `include/`: installed public headers such as `phev.h`, `phev_core.h`, `phev_service.h`.
- `test/`: Unity-based test sources plus `test/CMakeLists.txt` and the monolithic `test_runner.c`.
- `CMakeLists.txt`: root build definition for the static library and optional tests.
- `Dockerfile`: CI-like reproducible build that installs dependencies and runs `ctest`.
- `.github/workflows/dockerimage.yml`: GitHub Action that only builds the Docker image.

## Dependencies
- Runtime/build dependencies: `msg-core` and `cJSON`.
- Test dependency: Unity.
- README expects these libraries installed into standard locations such as `/usr/local`.
- `.gitmodules` references `external/Unity`, `external/msg-core`, and `external/cJSON`, but the submodule directories are not present in this checkout.

## Build Commands
- Configure release-ish local build:
```sh
cmake -S . -B build -DWANT_SPLINT=NO
```
- Build library:
```sh
cmake --build build
```
- Configure with tests enabled:
```sh
cmake -S . -B build -DBUILD_TESTS=ON -DWANT_SPLINT=NO
```
- Build everything including tests:
```sh
cmake --build build
```
- Install artifacts:
```sh
cmake --install build
```

## Test Commands
- Run all configured CTest tests:
```sh
ctest --test-dir build --output-on-failure
```
- Run the single registered CTest target:
```sh
ctest --test-dir build -R '^test_phev_core$' --output-on-failure
```
- Run the Unity runner directly:
```sh
./build/test/test_runner
```
- Run the Dockerized test flow used by CI/docs:
```sh
docker build -t phevcore . && docker run --rm phevcore
```
- Windows helper script:
```bat
runtests.bat
```

## Single-Test Guidance
- CTest only knows about one test target, `test_phev_core`, which actually executes the full `test_runner` binary.
- `test/test_runner.c` includes all test source files directly and calls many `RUN_TEST(...)` entries in one `main()`.
- There is no built-in per-test-name CLI filter wired into the current Unity runner.
- If you need one specific Unity case, the least invasive approach is to temporarily comment out unrelated `RUN_TEST(...)` lines in `test/test_runner.c`, build, run `./build/test/test_runner`, then restore the file.
- If you need repeatable focused execution, add a dedicated temporary runner in `test/` rather than reshaping production code.

## Lint / Static Analysis
- There is no ESLint/clang-format/Prettier-style tooling here; this is a plain C project.
- Root `CMakeLists.txt` includes `splint.cmake` and defines `WANT_SPLINT`, but `splint.cmake` is a stub.
- Prefer configuring with `-DWANT_SPLINT=NO` unless you also restore real Splint integration.
- There is no other repository-defined lint command.

## CI / Verification
- GitHub Actions currently validates that the Docker image builds; it does not run native host builds directly.
- The Dockerfile builds dependencies, configures with `-DBUILD_TESTS=true`, builds, and runs:
```sh
ctest -j6 -T test --output-on-failure
```
- For local verification after code changes, prefer:
```sh
cmake -S . -B build -DBUILD_TESTS=ON -DWANT_SPLINT=NO && cmake --build build && ctest --test-dir build --output-on-failure
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
- Opening braces usually go on the next line for functions and control statements.
- Keep one statement per line.
- Use spaces inside control keywords: `if (...)`, `switch (...)`, `for (...)`.
- Multi-line struct initializers commonly use one field per line with leading `.` designators.
- Keep line wrapping pragmatic; this codebase does not enforce a strict column limit.

## Naming Conventions
- Public and private functions use the `phev_` prefix.
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
- `test/test_runner.c` includes source-style test files with `#include "test_xxx.c"`; preserve that layout unless intentionally refactoring tests.
- Add new test functions to the relevant `test/test_*.c` file and wire them into `RUN_TEST(...)` in `test/test_runner.c`.
- Follow existing assertion style with Unity macros such as `TEST_ASSERT_EQUAL`, `TEST_ASSERT_NOT_NULL`, and `TEST_ASSERT_EQUAL_HEX8_ARRAY`.

## Editing Guidance For Agents
- Keep changes narrow and consistent with surrounding style; this is not a heavily normalized codebase.
- Prefer fixing bugs in-place over broad stylistic cleanup.
- Do not silently rename public macros, structs, or functions without updating all call sites and tests.
- Be careful around protocol constants and XOR/checksum logic in `phev_core.c`; small byte-level changes can invalidate many tests.
- Be careful around callback registration and pipe/service context wiring; several modules pass context through nested structs.

## Known Quirks To Respect
- `test/CMakeLists.txt` registers the executable as a single CTest case named `test_phev_core` even though it runs many suites.
- The project references Splint integration, but the helper CMake file is a stub in this checkout.
- Some code intentionally uses duplicated patterns, manual memory management, and verbose logging; preserve behavior first, elegance second.
- There are existing rough edges and probable bugs in the codebase; avoid opportunistic rewrites unless required for the task at hand.
