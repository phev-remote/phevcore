#define LOGGING_ON
#define LOG_LEVEL LOG_DEBUG
#define MY18

#include "unity.h"
#include "test_phev_service.c"

void setUp(void) { }
void tearDown(void) { }

int main(void)
{
    UNITY_BEGIN();

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
    RUN_TEST(test_phev_service_jsonCommandToPhevMessage_invalid_operation);
    RUN_TEST(test_phev_service_createPipe);
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

    return UNITY_END();
}
