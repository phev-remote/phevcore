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

# ---------- library ----------
add_library(phev STATIC
    # msg-core sources (merged from github.com/papawattu/msg-core)
    src/msg_core.c
    src/msg_mqtt.c
    src/msg_pipe.c
    src/msg_pipe_splitter.c
    src/msg_tcpip.c
    src/msg_utils.c
    # phev sources
    src/phev.c
    src/phev_config.c
    src/phev_core.c
    src/phev_model.c
    src/phev_pipe.c
    src/phev_register.c
    src/phev_service.c
    src/phev_tcpip.c
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
    PUBLIC cjson
)

if(WIN32)
    target_link_libraries(phev PUBLIC mswsock advapi32 ws2_32)
endif()

# ---------- tests ----------
option(BUILD_TESTS "Build the test binaries" OFF)

if(BUILD_TESTS)
    FetchContent_Declare(unity
        GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
        GIT_TAG        master
    )
    FetchContent_MakeAvailable(unity)

    enable_testing()
    add_subdirectory(test)
endif()

# ---------- install ----------
install(
    TARGETS phev
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES
    include/phev.h
    include/phev_config.h
    include/phev_core.h
    include/phev_model.h
    include/phev_pipe.h
    include/phev_register.h
    include/phev_service.h
    include/phev_tcpip.h
    include/msg_core.h
    include/msg_pipe.h
    include/msg_pipe_splitter.h
    include/msg_tcpip.h
    include/msg_utils.h
    include/logger.h
    DESTINATION include
)
