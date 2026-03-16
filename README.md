# Phev Remote Core Library

[![CI](https://github.com/phev-remote/phevcore/actions/workflows/dockerimage.yml/badge.svg?branch=main)](https://github.com/phev-remote/phevcore/actions/workflows/dockerimage.yml) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A C11 library for communicating with the Mitsubishi Outlander PHEV via the car's remote WiFi interface (GH4+ models with the MMC remote WiFi option).

Supports device registration, connecting to the car, reading vehicle data, and controlling lights and manual air conditioning.

The device must be connected to the car's network on the 192.168.8.x subnet.

### The CLI

A command-line tool built on this library, tested on Raspberry Pi and other Linux systems:

https://github.com/phev-remote/phevctl

### Dependencies

All dependencies are fetched automatically via CMake FetchContent:

- [cJSON](https://github.com/DaveGamble/cJSON) v1.7.18
- [greatest](https://github.com/silentbicycle/greatest) v1.5.0 (tests only)
- msg-core sources are vendored directly in `src/` and `include/`

### Build

Requires CMake >= 3.21 for preset support (>= 3.14 for manual configuration).

```sh
# Development (debug + tests)
cmake --preset dev
cmake --build --preset dev

# Release (optimized, no tests)
cmake --preset release
cmake --build --preset release
```

### Manual configuration

```sh
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build
```

### Test

```sh
# Using presets
ctest --preset dev

# Manual
ctest --test-dir build --output-on-failure
```

### Install

```sh
cmake --install build
```
