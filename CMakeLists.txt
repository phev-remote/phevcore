project("Phev Core" C)
cmake_minimum_required(VERSION 3.0)

set (CMAKE_C_STANDARD 11)

add_library(phev_core STATIC
    src/phev_register.c
    src/phev_pipe.c
    src/phev_core.c
    src/phev_service.c
)
target_link_libraries(phev_core msg_core cjson)

include(CTest)

add_subdirectory(external) 

target_include_directories(msg_core PUBLIC msg_core/include)

add_subdirectory(test)

target_include_directories(phev_core PUBLIC include)
