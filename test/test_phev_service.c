#include <stdbool.h>
#include "unity.h"
#include "phev_service.h"

void test_phev_service_outHandlerIn(messagingClient_t *client, message_t *message) 
{
    return;
}

message_t * test_phev_service_inHandlerIn(messagingClient_t *client) 
{
    return NULL;
}

void test_phev_service_outHandlerOut(messagingClient_t *client, message_t *message) 
{
    return;
}

message_t * test_phev_service_inHandlerOut(messagingClient_t *client) 
{
    message_t * message = NULL;
    //if(test_pipe_global_in_message) {
    //    message = msg_utils_copyMsg(test_pipe_global_in_message);
    //}
    
    return message;
}
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
void test_phev_service_validateCommand_updateRegister_multiple(void)
{
    const char * command = "{ \"updateRegister\" :  { \"reg\" : 1, \"value\" : 255 }, \"updateRegister\" :  { \"reg\" : 2, \"value\" : 255 } }";

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
void test_phev_service_jsonCommandToPhevMessage_airConOn(void)
{
    const char * command = "{ \"operation\" :  { \"airCon\" : \"on\" } }";
    
    phevMessage_t * message = phev_service_jsonCommandToPhevMessage(command);

    TEST_ASSERT_NOT_NULL(message);
    TEST_ASSERT_EQUAL(4,message->reg);
    TEST_ASSERT_EQUAL(1,message->data[0]);
    
}
void test_phev_service_jsonCommandToPhevMessage_airConOff(void)
{
    const char * command = "{ \"operation\" :  { \"airCon\" : \"off\" } }";
    
    phevMessage_t * message = phev_service_jsonCommandToPhevMessage(command);

    TEST_ASSERT_NOT_NULL(message);
    TEST_ASSERT_EQUAL(4,message->reg);
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
void test_phev_service_createPipe(void)
{
    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_service_inHandlerIn,
        .outgoingHandler = test_phev_service_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_service_inHandlerOut,
        .outgoingHandler = test_phev_service_outHandlerOut,
    };
    
    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phev_pipe_ctx_t * ctx = phev_service_createPipe(in,out);

    TEST_ASSERT_NOT_NULL(ctx);
}
void test_phev_service_jsonInputTransformer(void)
{
    const uint8_t expected[] = {0xf6,0x04,0x00,0x0a,0x01,0x05};
    const char * command = "{ \"operation\" :  { \"headLights\" : \"on\" } }";
    

    message_t * message = phev_service_jsonInputTransformer(NULL,msg_utils_createMsg(command, strlen(command)));

    TEST_ASSERT_NOT_NULL(message);
    TEST_ASSERT_EQUAL_MEMORY(expected, message->data, sizeof(expected));
}