#define LOGGING_ON
#define LOG_LEVEL LOG_DEBUG
#define MY18

#include "unity.h"
#include "test_phev_core.c"

void setUp(void) { }
void tearDown(void) { }

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_create_phev_message);
    RUN_TEST(test_destroy_phev_message);
    RUN_TEST(test_phev_core_extractAndDecodeIncomingMessageAndXOR);
    RUN_TEST(test_split_message_single_correct_return_val);
    RUN_TEST(test_split_message_single_correct_command);
    RUN_TEST(test_split_message_single_correct_length);
    RUN_TEST(test_split_message_single_correct_type);
    RUN_TEST(test_split_message_single_correct_reg);
    RUN_TEST(test_split_message_double_correct);
    RUN_TEST(test_split_message_double_decode);
    RUN_TEST(test_encode_message_single);
    RUN_TEST(test_encode_message_single_checksum);
    RUN_TEST(test_phev_core_encodeMessage);
    RUN_TEST(test_simple_command_request_message);
    RUN_TEST(test_simple_command_response_message);
    RUN_TEST(test_command_message);
    RUN_TEST(test_ack_message);
    RUN_TEST(test_start_encoded_message);
    RUN_TEST(test_response_handler_start);
    RUN_TEST(test_calc_checksum);
    RUN_TEST(test_phev_message_to_message);
    RUN_TEST(test_phev_ack_message);
    RUN_TEST(test_phev_head_lights_on);
    RUN_TEST(test_phev_head_lights_on_message);
    RUN_TEST(test_phev_core_copyMessage);
    RUN_TEST(test_response_handler_4e);
    RUN_TEST(test_phev_core_xor_message_even_xor_response);
    RUN_TEST(test_phev_core_xor_message_odd_xor_response);
    RUN_TEST(test_phev_core_xor_inbound_message_odd_xor_request);
    RUN_TEST(test_phev_core_xor_inbound_message_odd_ping);
    RUN_TEST(test_phev_core_xor_inbound_6f_resp);
    RUN_TEST(test_phev_core_xor_inbound_ping_even_resp);
    RUN_TEST(test_phev_core_getXOR_odd_request);
    RUN_TEST(test_phev_core_getXOR_even_request);
    RUN_TEST(test_phev_core_getXOR_odd_response);
    RUN_TEST(test_phev_core_getXOR_even_response);
    RUN_TEST(test_phev_core_getData);
    RUN_TEST(test_phev_core_decodeMessage_command_request);
    RUN_TEST(test_phev_core_decodeMessage_command_response);
    RUN_TEST(test_core_phev_core_extractIncomingMessageAndXOR_valid_ping_in_clear);
    RUN_TEST(test_core_phev_core_extractIncomingMessageAndXOR_valid_ping_encoded);
    RUN_TEST(test_core_phev_core_extractIncomingMessageAndXOR_valid_command_response_in_clear);
    RUN_TEST(test_core_phev_core_extractIncomingMessageAndXOR_valid_command_response_encoded);
    RUN_TEST(test_core_phev_core_extractIncomingMessageAndXOR_valid_command_request_in_clear);
    RUN_TEST(test_core_phev_core_extractIncomingMessageAndXOR_valid_command_request_encoded);
    RUN_TEST(test_core_phev_core_extractIncomingMessageAndXOR_valid_command_start_in_clear);
    RUN_TEST(test_core_phev_core_extractIncomingMessageAndXOR_invalid_command);
    RUN_TEST(test_core_phev_core_extractIncomingMessageAndXOR_BB_command);
    RUN_TEST(test_core_phev_core_extractIncomingMessageAndXOR_CC_command);
    RUN_TEST(test_core_phev_core_extractIncomingMessageAndXOR_2F_command);
    RUN_TEST(test_phev_core_getMessageXOR);
    RUN_TEST(test_core_phev_core_extractIncomingMessageValidFirstByteCommand);

    return UNITY_END();
}
