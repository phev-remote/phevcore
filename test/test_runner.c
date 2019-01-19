//#define LOGGING_OFF
//#define TEST_TIMEOUTS

#include "unity.h"
#include "test_phev_core.c"
#include "test_phev_register.c"
#include "test_phev_pipe.c"
#include "test_phev_service.c"
#include "test_phev_model.c"


int main()
{
    UNITY_BEGIN();
//  PHEV_CORE

    RUN_TEST(test_create_phev_message);
    RUN_TEST(test_destroy_phev_message);
    RUN_TEST(test_split_message_single_correct_size);
    RUN_TEST(test_split_message_single_correct_command);
    RUN_TEST(test_split_message_single_correct_length);
    RUN_TEST(test_split_message_single_correct_type);
    RUN_TEST(test_split_message_single_correct_reg);
    RUN_TEST(test_split_message_single_correct_data);
    RUN_TEST(test_split_message_double_correct);
    RUN_TEST(test_split_message_double_decode);
    RUN_TEST(test_encode_message_single);
    RUN_TEST(test_encode_message_single_checksum);
    RUN_TEST(test_simple_command_request_message);
    RUN_TEST(test_simple_command_response_message);
    RUN_TEST(test_command_message);
    RUN_TEST(test_ack_message);
    RUN_TEST(test_start_message);
    RUN_TEST(test_start_encoded_message);
    RUN_TEST(test_ping_message);
    RUN_TEST(test_response_handler_start);
    RUN_TEST(test_calc_checksum);
    RUN_TEST(test_phev_message_to_message);
    RUN_TEST(test_phev_ack_message); 
    RUN_TEST(test_phev_head_lights_on);
    RUN_TEST(test_phev_head_lights_on_message);
    RUN_TEST(test_phev_mac_response);
    RUN_TEST(test_phev_message_to_phev_message_and_back);
    RUN_TEST(test_phev_core_copyMessage);

//  PHEV_REGISTER

    RUN_TEST(test_phev_register_bootstrap);
    RUN_TEST(test_phev_register_getVin);
    RUN_TEST(test_phev_register_should_error_when_too_many_registrations);
    RUN_TEST(test_phev_register_should_send_mac_and_aa);
    RUN_TEST(test_phev_register_should_trigger_aa_ack_event);
    RUN_TEST(test_phev_register_should_call_complete_when_registered);
    RUN_TEST(test_phev_register_should_get_start_ack);
    RUN_TEST(test_phev_register_should_get_aa_ack);
    RUN_TEST(test_phev_register_should_get_registration);
    RUN_TEST(test_phev_register_should_get_ecu_version);
    RUN_TEST(test_phev_register_should_get_remote_security_present);
    RUN_TEST(test_phev_register_should_get_reg_disp);
    RUN_TEST(test_phev_register_end_to_end);

//  PHEV PIPE
    
    RUN_TEST(test_phev_pipe_createPipe);
    RUN_TEST(test_phev_pipe_create);
    RUN_TEST(test_phev_pipe_loop);
    RUN_TEST(test_phev_pipe_start);
    RUN_TEST(test_phev_pipe_outputChainInputTransformer);
    RUN_TEST(test_phev_pipe_splitter_one_message);
    RUN_TEST(test_phev_pipe_publish);
    RUN_TEST(test_phev_pipe_commandResponder);
    RUN_TEST(test_phev_pipe_commandResponder_should_only_respond_to_commands);
    RUN_TEST(test_phev_pipe_no_input_connection);
#ifdef TEST_TIMEOUTS
    RUN_TEST(test_phev_pipe_waitForConnection_should_timeout);
    RUN_TEST(test_phev_pipe_waitForConnection);
#endif
    RUN_TEST(test_phev_pipe_sendMac);
    RUN_TEST(test_phev_pipe_updateRegister);
    RUN_TEST(test_phev_pipe_updateRegisterWithCallback);
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
    RUN_TEST(test_phev_service_validateCommand_updateRegister_reg_out_of_range);
    RUN_TEST(test_phev_service_validateCommand_updateRegister_value_out_of_range);
    RUN_TEST(test_phev_service_jsonCommandToPhevMessage_updateRegister);
    RUN_TEST(test_phev_service_jsonCommandToPhevMessage_headLightsOn);
    RUN_TEST(test_phev_service_jsonCommandToPhevMessage_headLightsOff);
    RUN_TEST(test_phev_service_jsonCommandToPhevMessage_headLights_invalidValue);
    RUN_TEST(test_phev_service_jsonCommandToPhevMessage_airConOn);
    RUN_TEST(test_phev_service_jsonCommandToPhevMessage_airConOff);
    RUN_TEST(test_phev_service_jsonCommandToPhevMessage_invalid_operation);
    RUN_TEST(test_phev_service_createPipe);
    RUN_TEST(test_phev_service_jsonInputTransformer);
    RUN_TEST(test_phev_service_jsonOutputTransformer_updated_register); 
    RUN_TEST(test_phev_service_jsonOutputTransformer_updated_register_reg);
    RUN_TEST(test_phev_service_jsonOutputTransformer_updated_register_length);
    RUN_TEST(test_phev_service_jsonOutputTransformer_updated_register_data);
    RUN_TEST(test_phev_service_jsonOutputTransformer_updated_register_data_multiple_items);
    RUN_TEST(test_phev_service_jsonOutputTransformer_updated_register_ack);
    RUN_TEST(test_phev_service_jsonOutputTransformer_updated_register_ack_register);
    RUN_TEST(test_phev_service_jsonOutputTransformer_not_updated_register);
    RUN_TEST(test_phev_service_jsonOutputTransformer_has_updated_register);
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
    RUN_TEST(test_phev_service_register);
    RUN_TEST(test_phev_service_register_complete_called);

//  PHEV_MODEL

    RUN_TEST(test_phev_model_create_model);
    RUN_TEST(test_phev_model_set_register);
    RUN_TEST(test_phev_model_get_register);
    RUN_TEST(test_phev_model_get_register_not_found);
    RUN_TEST(test_phev_model_update_register);
    RUN_TEST(test_phev_model_register_compare);
    RUN_TEST(test_phev_model_register_compare_not_same);
    RUN_TEST(test_phev_model_compare_not_set);

    return UNITY_END();
}