add_library(Unity STATIC
    Unity/src/unity.c
)

target_include_directories(Unity PUBLIC
    Unity/src
)

add_library(msg_core STATIC
    msg-core/src/msg_core.c
    msg-core/src/msg_gcp_mqtt.c
    msg-core/src/msg_mqtt.c
    msg-core/src/msg_pipe_splitter.c
    msg-core/src/msg_pipe.c 
    msg-core/src/msg_tcpip.c 
    msg-core/src/msg_utils.c
)

target_include_directories(msg_core PUBLIC msg-core/include)

add_library(cjson STATIC cJSON/cJSON.c)
target_include_directories(cjson PUBLIC cJSON)
