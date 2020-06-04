#define LOGGING_ON
#define LOG_LEVEL LOG_DEBUG
//#define TEST_TIMEOUTS

#define MY18

#include "unity.h"
#include "test_phev_core.c"
#include "test_phev_register.c"
#include "test_phev_pipe.c"
#include "test_phev_service.c"
#include "test_phev_model.c"
#include "test_phev.c"

void setUp(void) 
{

}
void tearDown(void)
{

}
int main()
{
    UNITY_BEGIN();

//  PHEV_CORE

    RUN_TEST(test_create_phev_message);
    RUN_TEST(test_destroy_phev_message);
    RUN_TEST(test_phev_core_extractAndDecodeIncomingMessageAndXOR);
    RUN_TEST(test_split_message_single_correct_return_val);
    RUN_TEST(test_split_message_single_correct_command);
    RUN_TEST(test_split_message_single_correct_length);
    RUN_TEST(test_split_message_single_correct_type);
    RUN_TEST(test_split_message_single_correct_reg);
//    RUN_TEST(test_split_message_single_correct_data);
    RUN_TEST(test_split_message_double_correct);
    RUN_TEST(test_split_message_double_decode);
    RUN_TEST(test_encode_message_single);
    RUN_TEST(test_encode_message_single_checksum);
    RUN_TEST(test_phev_core_encodeMessage);
//    RUN_TEST(test_phev_core_encodeMessage_encoded);
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

//  PHEV PIPE
    
    RUN_TEST(test_phev_pipe_loop);

//    RUN_TEST(test_phev_pipe_outputChainInputTransformer);
//    RUN_TEST(test_phev_pipe_outputChainInputTransformer_encoded);
//    RUN_TEST(test_phev_pipe_outputChainInputTransformer_changedXOR_command_response);
//    RUN_TEST(test_phev_pipe_outputChainInputTransformer_changedXOR_command_request);
//    RUN_TEST(test_phev_pipe_outputChainInputTransformer_changedXOR_ping_response);
    RUN_TEST(test_phev_pipe_splitter_one_message);
    RUN_TEST(test_phev_pipe_splitter_two_messages);


    RUN_TEST(test_phev_pipe_splitter_one_encoded_message);
    RUN_TEST(test_phev_pipe_splitter_two_encoded_messages);

    RUN_TEST(test_phev_pipe_publish);
    RUN_TEST(test_phev_pipe_commandResponder);
//    RUN_TEST(test_phev_pipe_commandResponder_reg_update_odd_xor);
//    RUN_TEST(test_phev_pipe_commandResponder_reg_update_even_xor);
    RUN_TEST(test_phev_pipe_ping_even_xor);
    RUN_TEST(test_phev_pipe_ping_odd_xor);
    RUN_TEST(test_phev_pipe_commandResponder_should_only_respond_to_commands);
//    RUN_TEST(test_phev_pipe_commandResponder_should_encrypt_with_correct_xor);
    RUN_TEST(test_phev_pipe_no_input_connection);
#ifdef TEST_TIMEOUTS
    RUN_TEST(test_phev_pipe_waitForConnection_should_timeout);
    RUN_TEST(test_phev_pipe_waitForConnection);
#endif
//    RUN_TEST(test_phev_pipe_sendMac);
    RUN_TEST(test_phev_pipe_updateRegister);
    RUN_TEST(test_phev_pipe_updateRegisterWithCallback);
    RUN_TEST(test_phev_pipe_updateRegisterWithCallback_encoded);
    RUN_TEST(test_phev_pipe_registerEventHandler);
    RUN_TEST(test_phev_pipe_register_multiple_registerEventHandlers);
    RUN_TEST(test_phev_pipe_createRegisterEvent_ack);
    RUN_TEST(test_phev_pipe_createRegisterEvent_update);    

// PHEV SERVICE

    RUN_TEST(test_phev_service_validateCommand);
    RUN_TEST(test_phev_service_validateCommand_empty);
    RUN_TEST(test_phev_service_validateCommand_invalidJson);
    RUN_TEST(test_phev_service_validateCommand_updateRegister_invalid);
    RUN_TEST(test_phev_service_validateCommand_updateRegister_valid);
    RUN_TEST(test_phev_service_validateCommand_updateRegister_multiple);
    RUN_TEST(test_phev_service_validateCommand_updateRegister_data_array);
    RUN_TEST(test_phev_service_validateCommand_updateRegister_data_array_invalid);
    RUN_TEST(test_phev_service_validateCommand_updateRegister_reg_out_of_range);
    RUN_TEST(test_phev_service_validateCommand_updateRegister_value_out_of_range);
    RUN_TEST(test_phev_service_jsonCommandToPhevMessage_updateRegister);
    RUN_TEST(test_phev_service_jsonCommandToPhevMessage_updateRegister_data_array);
    RUN_TEST(test_phev_service_jsonCommandToPhevMessage_updateRegister_data_array_invalid);
    RUN_TEST(test_phev_service_jsonCommandToPhevMessage_headLightsOn);
    RUN_TEST(test_phev_service_jsonCommandToPhevMessage_headLightsOff);
    RUN_TEST(test_phev_service_jsonCommandToPhevMessage_headLights_invalidValue);
    RUN_TEST(test_phev_service_jsonCommandToPhevMessage_airConOn);
    RUN_TEST(test_phev_service_jsonCommandToPhevMessage_airConOff);
    RUN_TEST(test_phev_service_jsonCommandToPhevMessage_airConOn_windscreen);
    RUN_TEST(test_phev_service_jsonCommandToPhevMessage_airConOn_heat);
    RUN_TEST(test_phev_service_jsonCommandToPhevMessage_airConOn_cool);
    //RUN_TEST(test_phev_service_jsonCommandToPhevMessage_update);
    RUN_TEST(test_phev_service_jsonCommandToPhevMessage_invalid_operation);
    RUN_TEST(test_phev_service_createPipe);
    //RUN_TEST(test_phev_service_jsonInputTransformer);
    //RUN_TEST(test_phev_service_jsonOutputTransformer_updated_register); 
    //RUN_TEST(test_phev_service_jsonOutputTransformer_updated_register_reg);
    //RUN_TEST(test_phev_service_jsonOutputTransformer_updated_register_length);
    //RUN_TEST(test_phev_service_jsonOutputTransformer_updated_register_data);
    //RUN_TEST(test_phev_service_jsonOutputTransformer_updated_register_data_multiple_items);
    //RUN_TEST(test_phev_service_jsonOutputTransformer_updated_register_ack);
    //RUN_TEST(test_phev_service_jsonOutputTransformer_updated_register_ack_register);
    //RUN_TEST(test_phev_service_jsonOutputTransformer_not_updated_register);
    //RUN_TEST(test_phev_service_jsonOutputTransformer_has_updated_register);
    RUN_TEST(test_phev_service_init);
    RUN_TEST(test_phev_service_get_battery_level);
    RUN_TEST(test_phev_service_get_battery_level_not_set);
    RUN_TEST(test_phev_service_statusAsJson);
    RUN_TEST(test_phev_service_statusAsJson_has_status_object);
    RUN_TEST(test_phev_service_statusAsJson_has_battery_object);
    RUN_TEST(test_phev_service_statusAsJson_has_no_battery_level);
    RUN_TEST(test_phev_service_statusAsJson_has_battery_level_correct);
    RUN_TEST(test_phev_service_outputFilter);
    RUN_TEST(test_phev_service_outputFilter_no_change);
    RUN_TEST(test_phev_service_outputFilter_change);
    RUN_TEST(test_phev_service_inputSplitter_not_null);
    RUN_TEST(test_phev_service_inputSplitter_two_messages_num_messages);
    RUN_TEST(test_phev_service_inputSplitter_two_messages_first);
    RUN_TEST(test_phev_service_inputSplitter_two_messages_second);
    RUN_TEST(test_phev_service_end_to_end_operations);
    RUN_TEST(test_phev_service_end_to_end_updated_register);
    RUN_TEST(test_phev_service_end_to_end_multiple_updated_registers);
    RUN_TEST(test_phev_service_jsonResponseAggregator);
    RUN_TEST(test_phev_service_init_settings);
    RUN_TEST(test_phev_service_register_complete_called);
    RUN_TEST(test_phev_service_register_complete_resets_transformers);
    RUN_TEST(test_phev_service_create);
    RUN_TEST(test_phev_service_getRegister);
    RUN_TEST(test_phev_service_setRegister);
    RUN_TEST(test_phev_service_getRegisterJson);
    RUN_TEST(test_phev_service_create_passes_context);
    RUN_TEST(test_phev_service_getDateSync);
    RUN_TEST(test_phev_service_statusAsJson_dateSync);
    RUN_TEST(test_phev_service_statusAsJson_not_charging);
    RUN_TEST(test_phev_service_statusAsJson_is_charging);
    RUN_TEST(test_phev_service_hvacStatus_on);
    RUN_TEST(test_phev_service_hvacStatus_off);
    RUN_TEST(test_phev_service_statusAsJson_hvac_operating);
    RUN_TEST(test_phev_service_status);
    
//  PHEV_MODEL

    RUN_TEST(test_phev_model_create_model);
    RUN_TEST(test_phev_model_set_register);
    RUN_TEST(test_phev_model_get_register);
    RUN_TEST(test_phev_model_get_register_not_found);
    RUN_TEST(test_phev_model_update_register);
    RUN_TEST(test_phev_model_register_compare);
    RUN_TEST(test_phev_model_register_compare_not_same);
    RUN_TEST(test_phev_model_compare_not_set);

// PHEV

    RUN_TEST(test_phev_init_returns_context);
    RUN_TEST(test_phev_statusAsJson);
   // RUN_TEST(test_phev_calls_connect_event);
   // RUN_TEST(test_phev_registrationEndToEnd);

    return UNITY_END();

}