#include <stdbool.h>
#include "unity.h"
#include "phev_service.h"

void test_phev_service_validateCommand(void)
{
    const char * command = "{ \"updateRegister\" : { \"reg\" : 1, \"value\" : 255 } }";

    bool ret = phev_service_validateCommand(command);

    TEST_ASSERT_TRUE(ret);
}

void test_phev_service_validateCommand_empty(void)
{
    const char * command = "";

    bool ret = phev_service_validateCommand(command);

    TEST_ASSERT_FALSE(ret);
}

void test_phev_service_validateCommand_invalidJson(void)
{
    const char * command = "{ \"updateRegister\" :  }";

    bool ret = phev_service_validateCommand(command);

    TEST_ASSERT_FALSE(ret);
}

void test_phev_service_validateCommand_updateRegister_invalid(void)
{
    const char * command = "{ \"updateRegister\" :  { \"someValue\" : 123 } }";

    bool ret = phev_service_validateCommand(command);

    TEST_ASSERT_FALSE(ret);
}
void test_phev_service_validateCommand_updateRegister_valid(void)
{
    const char * command = "{ \"updateRegister\" :  { \"reg\" : 1, \"value\" : 255 } }";

    bool ret = phev_service_validateCommand(command);

    TEST_ASSERT_TRUE(ret);
}
void test_phev_service_validateCommand_updateRegister_reg_out_of_range(void)
{
    const char * command = "{ \"updateRegister\" :  { \"reg\" : 555, \"value\" : 255 } }";

    bool ret = phev_service_validateCommand(command);

    TEST_ASSERT_FALSE(ret);
}
void test_phev_service_validateCommand_updateRegister_value_out_of_range(void)
{
    const char * command = "{ \"updateRegister\" :  { \"reg\" : 1, \"value\" : 256 } }";

    bool ret = phev_service_validateCommand(command);

    TEST_ASSERT_FALSE(ret);
}
void test_phev_service_jsonCommandToPhevMessage_updateRegister(void)
{
    const char * command = "{ \"updateRegister\" :  { \"reg\" : 1, \"value\" : 255 } }";
    
    phevMessage_t * message = phev_service_jsonCommandToPhevMessage(command);

    TEST_ASSERT_NOT_NULL(message);
    TEST_ASSERT_EQUAL(message->reg, 1);
    TEST_ASSERT_EQUAL(message->data[0], 255);
    
}
void test_phev_service_jsonCommandToPhevMessage_headLightsOn(void)
{
    const char * command = "{ \"operation\" :  { \"headLights\" : \"on\" } }";
    
    phevMessage_t * message = phev_service_jsonCommandToPhevMessage(command);

    TEST_ASSERT_NOT_NULL(message);
    TEST_ASSERT_EQUAL(10,message->reg);
    TEST_ASSERT_EQUAL(1,message->data[0]);
    
}
void test_phev_service_jsonCommandToPhevMessage_headLightsOff(void)
{
    const char * command = "{ \"operation\" :  { \"headLights\" : \"off\" } }";
    
    phevMessage_t * message = phev_service_jsonCommandToPhevMessage(command);

    TEST_ASSERT_NOT_NULL(message);
    TEST_ASSERT_EQUAL(10,message->reg);
    TEST_ASSERT_EQUAL(2,message->data[0]);
    
}
void test_phev_service_jsonCommandToPhevMessage_headLights_invalidValue(void)
{
    const char * command = "{ \"operation\" :  { \"headLights\" : \"dim\" } }";
    
    phevMessage_t * message = phev_service_jsonCommandToPhevMessage(command);

    TEST_ASSERT_NULL(message);
}
void test_phev_service_jsonCommandToPhevMessage_invalid_operation(void)
{
    const char * command = "{ \"operation\" :  { \"abc\" : \"off\" } }";
    
    phevMessage_t * message = phev_service_jsonCommandToPhevMessage(command);

    TEST_ASSERT_NULL(message);
    
}