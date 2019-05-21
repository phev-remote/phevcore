project("Phev" C)
cmake_minimum_required(VERSION 3.0)

set (CMAKE_C_STANDARD 11)

add_library(phev STATIC
    src/phev_register.c
    src/phev_pipe.c
    src/phev_core.c
    src/phev_service.c
    src/phev_model.c
    src/phev_tcpip.c
    src/phev.c
)
# target_link_libraries(phev msg_core cjson)

include(CTest)

add_subdirectory(external) 

target_include_directories(msg_core PUBLIC msg_core/include)

add_subdirectory(test)

if(WIN32)
    target_link_libraries(phev LINK_PUBLIC
        msg_core
        cjson
        mswsock
        advapi32 
        ws2_32

    )
else()
    target_link_libraries (phev LINK_PUBLIC 
        msg_core
        cjson  
    )
endif()

target_include_directories(phev PUBLIC include)
