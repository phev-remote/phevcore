#define LOGGING_ON
#define LOG_LEVEL LOG_DEBUG
#define MY18

#include "unity.h"
#include "test_phev_pipe.c"

void setUp(void) { }
void tearDown(void) { }

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_phev_pipe_loop);
    RUN_TEST(test_phev_pipe_splitter_one_message);
    RUN_TEST(test_phev_pipe_splitter_two_messages);
    RUN_TEST(test_phev_pipe_splitter_one_encoded_message);
    RUN_TEST(test_phev_pipe_splitter_two_encoded_messages);
    RUN_TEST(test_phev_pipe_publish);
    RUN_TEST(test_phev_pipe_commandResponder);
    RUN_TEST(test_phev_pipe_ping_even_xor);
    RUN_TEST(test_phev_pipe_ping_odd_xor);
    RUN_TEST(test_phev_pipe_commandResponder_should_only_respond_to_commands);
    RUN_TEST(test_phev_pipe_no_input_connection);
    RUN_TEST(test_phev_pipe_updateRegister);
    RUN_TEST(test_phev_pipe_updateRegisterWithCallback);
    RUN_TEST(test_phev_pipe_updateRegisterWithCallback_encoded);
    RUN_TEST(test_phev_pipe_registerEventHandler);
    RUN_TEST(test_phev_pipe_register_multiple_registerEventHandlers);
    RUN_TEST(test_phev_pipe_createRegisterEvent_ack);
    RUN_TEST(test_phev_pipe_createRegisterEvent_update);

    return UNITY_END();
}
