cmake_minimum_required(VERSION 3.14)
project(phev VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(UNIX)
    add_definitions(-D__unix__)
endif()

# ---------- dependencies via FetchContent ----------
include(FetchContent)

FetchContent_Declare(cjson
    GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
    GIT_TAG        v1.7.18
)

# Disable cJSON extras we don't need
set(ENABLE_CJSON_TEST OFF CACHE BOOL "" FORCE)
set(ENABLE_CJSON_UTILS OFF CACHE BOOL "" FORCE)
set(CJSON_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(cjson)

# ---------- msg_core library (vendored from github.com/papawattu/msg-core) ----------
add_library(msg_core STATIC
    src/msg/msg_core.c
    src/msg/msg_mqtt.c
    src/msg/msg_pipe.c
    src/msg/msg_pipe_splitter.c
    src/msg/msg_tcpip.c
    src/msg/msg_utils.c
)

target_include_directories(msg_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(WIN32)
    target_link_libraries(msg_core PUBLIC mswsock advapi32 ws2_32)
endif()

# ---------- optional dead transport backends ----------
option(BUILD_TRANSPORT_BACKENDS "Build msg_gcp_mqtt and msg_mqtt_paho (requires external deps)" OFF)

if(BUILD_TRANSPORT_BACKENDS)
    target_sources(msg_core PRIVATE
        src/msg/msg_gcp_mqtt.c
        src/msg/msg_mqtt_paho.c
    )
endif()

# ---------- phev library ----------
add_library(phev STATIC
    src/phev/phev.c
    src/phev/phev_config.c
    src/phev/phev_core.c
    src/phev/phev_model.c
    src/phev/phev_pipe.c
    src/phev/phev_register.c
    src/phev/phev_service.c
    src/phev/phev_tcpip.c
)

target_include_directories(phev
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${cjson_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(phev
    PUBLIC msg_core cjson
)

# ---------- tests ----------
option(BUILD_TESTS "Build the test binaries" OFF)

if(BUILD_TESTS)
    FetchContent_Declare(greatest
        GIT_REPOSITORY https://github.com/silentbicycle/greatest.git
        GIT_TAG        v1.5.0
    )
    FetchContent_MakeAvailable(greatest)

    enable_testing()
    add_subdirectory(test)
endif()

# ---------- install ----------
install(
    TARGETS msg_core phev
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES
    include/phev/phev.h
    include/phev/phev_config.h
    include/phev/phev_core.h
    include/phev/phev_model.h
    include/phev/phev_pipe.h
    include/phev/phev_register.h
    include/phev/phev_service.h
    include/phev/phev_tcpip.h
    DESTINATION include/phev
)

install(FILES
    include/msg/msg_core.h
    include/msg/msg_pipe.h
    include/msg/msg_pipe_splitter.h
    include/msg/msg_tcpip.h
    include/msg/msg_utils.h
    include/msg/msg_mqtt.h
    include/msg/logger.h
    DESTINATION include/msg
)

if(BUILD_TRANSPORT_BACKENDS)
    install(FILES
        include/msg/msg_gcp_mqtt.h
        include/msg/msg_mqtt_paho.h
        include/msg/config.h
        DESTINATION include/msg
    )
endif()
