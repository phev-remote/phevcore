#include <stdbool.h>
#include "unity.h"
#include "cJSON.h"
#include "phev_service.h"

message_t * test_phev_service_global_in_in_message = NULL;
message_t * test_phev_service_global_in_out_message = NULL;
message_t * test_phev_service_global_out_in_message = NULL;
message_t * test_phev_service_global_out_out_message = NULL;


void test_phev_service_outHandlerIn(messagingClient_t *client, message_t *message) 
{
    test_phev_service_global_out_in_message = msg_utils_copyMsg(message);
    return;
}

message_t * test_phev_service_inHandlerIn(messagingClient_t *client) 
{
    message_t * message = NULL;
    if(test_phev_service_global_in_in_message) {
        message = msg_utils_copyMsg(test_phev_service_global_in_in_message);
    }
    return message;
}
void test_phev_service_outHandlerOut(messagingClient_t *client, message_t *message) 
{
    test_phev_service_global_out_out_message = msg_utils_copyMsg(message);
    return;
}

message_t * test_phev_service_inHandlerOut(messagingClient_t *client) 
{
    message_t * message = NULL;
    if(test_phev_service_global_in_out_message) {
        message = msg_utils_copyMsg(test_phev_service_global_in_out_message);
    }
    
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
    TEST_ASSERT_EQUAL(2,message->data[0]);
    
}
void test_phev_service_jsonCommandToPhevMessage_airConOff(void)
{
    const char * command = "{ \"operation\" :  { \"airCon\" : \"off\" } }";
    
    phevMessage_t * message = phev_service_jsonCommandToPhevMessage(command);

    TEST_ASSERT_NOT_NULL(message);
    TEST_ASSERT_EQUAL(4,message->reg);
    TEST_ASSERT_EQUAL(1,message->data[0]);
    
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

    phev_pipe_ctx_t * ctx = phev_service_createPipe(NULL,in,out);

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
void test_phev_service_jsonOutputTransformer_updated_register(void)
{
    const uint8_t message[] = {0x6f,0x04,0x00,0x0a,0x00,0x05};
    
    message_t * out = phev_service_jsonOutputTransformer(NULL,msg_utils_createMsg(message, sizeof(message)));

    TEST_ASSERT_NOT_NULL(out);
    
    const cJSON * outputedJson = cJSON_Parse(out->data);

    TEST_ASSERT_NOT_NULL(outputedJson);
    TEST_ASSERT_NOT_NULL(cJSON_GetObjectItemCaseSensitive(outputedJson,"updatedRegister"));
    
} // Filter now does this
void test_phev_service_jsonOutputTransformer_not_updated_register(void)
{
    const uint8_t message[] = {0x6f,0x04,0x00,0x0a,0x01,0x05};
    
    const uint8_t data[] = {1};
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

    phevServiceCtx_t * ctx = phev_service_init(in,out);
    phev_model_setRegister(ctx->model,10,data,1);
    
    message_t * outmsg = phev_service_jsonOutputTransformer(ctx->pipe,msg_utils_createMsg(message, sizeof(message)));

    TEST_ASSERT_NULL(outmsg);
    
} 
void test_phev_service_jsonOutputTransformer_has_updated_register(void)
{
    const uint8_t message[] = {0x6f,0x04,0x00,0x0a,0x02,0x05};
    
    const uint8_t data[] = {1};
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

    phevServiceCtx_t * ctx = phev_service_init(in,out);
    phev_model_setRegister(ctx->model,10,data,1);
    
    message_t * outmsg = phev_service_jsonOutputTransformer(ctx->pipe,msg_utils_createMsg(message, sizeof(message)));

    TEST_ASSERT_NOT_NULL(outmsg);
    
}
void test_phev_service_jsonOutputTransformer_updated_register_reg(void)
{
    const uint8_t message[] = {0x6f,0x04,0x00,0x0a,0x00,0x05};
    
    message_t * out = phev_service_jsonOutputTransformer(NULL,msg_utils_createMsg(message, sizeof(message)));

    TEST_ASSERT_NOT_NULL(out);
    
    const cJSON * outputedJson = cJSON_Parse(out->data);
    const cJSON * updatedRegister = cJSON_GetObjectItemCaseSensitive(outputedJson,"updatedRegister");
    const cJSON * reg = cJSON_GetObjectItemCaseSensitive(updatedRegister,"register");

    TEST_ASSERT_NOT_NULL(reg);
    TEST_ASSERT_EQUAL(10,reg->valueint);
    
}
void test_phev_service_jsonOutputTransformer_updated_register_length(void)
{
    const uint8_t message[] = {0x6f,0x04,0x00,0x0a,0x00,0x05};
    
    message_t * out = phev_service_jsonOutputTransformer(NULL,msg_utils_createMsg(message, sizeof(message)));

    TEST_ASSERT_NOT_NULL(out);
    
    const cJSON * outputedJson = cJSON_Parse(out->data);
    
    TEST_ASSERT_NOT_NULL(outputedJson);

    const cJSON * updatedRegister = cJSON_GetObjectItemCaseSensitive(outputedJson,"updatedRegister");
    
    TEST_ASSERT_NOT_NULL(updatedRegister);
    
    const cJSON * length = cJSON_GetObjectItemCaseSensitive(updatedRegister,"length");

    TEST_ASSERT_NOT_NULL(length);

    TEST_ASSERT_EQUAL(1,length->valueint);
    
}
void test_phev_service_jsonOutputTransformer_updated_register_data(void)
{
    const uint8_t message[] = {0x6f,0x04,0x00,0x0a,0xff,0x05};
    
    message_t * out = phev_service_jsonOutputTransformer(NULL,msg_utils_createMsg(message, sizeof(message)));

    TEST_ASSERT_NOT_NULL(out);
    
    const cJSON * item = NULL;
    const cJSON * outputedJson = cJSON_Parse(out->data);

    TEST_ASSERT_NOT_NULL(outputedJson);
    
    const cJSON * updatedRegister = cJSON_GetObjectItemCaseSensitive(outputedJson,"updatedRegister");
    
    TEST_ASSERT_NOT_NULL(updatedRegister);
    
    const cJSON * data = cJSON_GetObjectItemCaseSensitive(updatedRegister,"data");

    int i = 0;

    TEST_ASSERT_NOT_NULL(data);
    
    cJSON_ArrayForEach(item, data)
    {
        TEST_ASSERT_NOT_NULL(item);
        TEST_ASSERT_EQUAL(255,item->valueint);
        i++;
    }
    TEST_ASSERT_EQUAL(1,i);
}
void test_phev_service_jsonOutputTransformer_updated_register_data_multiple_items(void)
{
    const uint8_t numbers[] = {0xff,0xcc,0x55};
    const uint8_t message[] = {0x6f,0x06,0x00,0x0a,0xff,0xcc,0x55,0x05};
    
    message_t * out = phev_service_jsonOutputTransformer(NULL,msg_utils_createMsg(message, sizeof(message)));

    TEST_ASSERT_NOT_NULL(out);
    
    const cJSON * item = NULL;
    const cJSON * outputedJson = cJSON_Parse(out->data);

    TEST_ASSERT_NOT_NULL(outputedJson);
    
    const cJSON * updatedRegister = cJSON_GetObjectItemCaseSensitive(outputedJson,"updatedRegister");
    
    TEST_ASSERT_NOT_NULL(updatedRegister);

    const cJSON * data = cJSON_GetObjectItemCaseSensitive(updatedRegister,"data");
    int i = 0;

    TEST_ASSERT_NOT_NULL(data);
    
    cJSON_ArrayForEach(item, data)
    {
        TEST_ASSERT_NOT_NULL(item);
        TEST_ASSERT_EQUAL(numbers[i++],item->valueint);
    }
    TEST_ASSERT_EQUAL(i,3);
    
}
void test_phev_service_jsonOutputTransformer_updated_register_ack(void)
{
    const uint8_t message[] = {0x6f,0x04,0x01,0x0a,0x00,0x05};
    
    message_t * out = phev_service_jsonOutputTransformer(NULL,msg_utils_createMsg(message, sizeof(message)));

    TEST_ASSERT_NOT_NULL(out);
    
    const cJSON * outputedJson = cJSON_Parse(out->data);

    TEST_ASSERT_NOT_NULL(outputedJson);
    TEST_ASSERT_NOT_NULL(cJSON_GetObjectItemCaseSensitive(outputedJson,"updateRegisterAck"));
    
}
void test_phev_service_jsonOutputTransformer_updated_register_ack_register(void)
{
    const uint8_t message[] = {0x6f,0x04,0x01,0x0a,0x00,0x05};
    
    message_t * out = phev_service_jsonOutputTransformer(NULL,msg_utils_createMsg(message, sizeof(message)));

    TEST_ASSERT_NOT_NULL(out);
    
    const cJSON * outputedJson = cJSON_Parse(out->data);
    
    TEST_ASSERT_NOT_NULL(outputedJson);
    
    const cJSON * updatedRegisterAck = cJSON_GetObjectItemCaseSensitive(outputedJson,"updateRegisterAck");
    
    TEST_ASSERT_NOT_NULL(updatedRegisterAck);
    
    const cJSON * reg = cJSON_GetObjectItemCaseSensitive(updatedRegisterAck,"register");

    TEST_ASSERT_NOT_NULL(reg);
    TEST_ASSERT_EQUAL(0x0a, reg->valueint);
    
}
void test_phev_service_init(void)
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

    phevServiceCtx_t * ctx = phev_service_init(in,out);

    TEST_ASSERT_NOT_NULL(ctx);
    TEST_ASSERT_NOT_NULL(ctx->model);
    
}
void test_phev_service_get_battery_level()
{
    const uint8_t data[] = {50};
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

    phevServiceCtx_t * ctx = phev_service_init(in,out);

    phev_model_setRegister(ctx->model,29,data,1);
    int level = phev_service_getBatteryLevel(ctx);

    TEST_ASSERT_EQUAL(50,level);
}
void test_phev_service_get_battery_level_not_set()
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

    phevServiceCtx_t * ctx = phev_service_init(in,out);
    int level = phev_service_getBatteryLevel(ctx);

    TEST_ASSERT_EQUAL(-1,level);
}
void test_phev_service_statusAsJson()
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

    phevServiceCtx_t * ctx = phev_service_init(in,out);
    char * str = phev_service_statusAsJson(ctx);
    
    cJSON * json = cJSON_Parse(str);

    TEST_ASSERT_NOT_NULL(json);

}
void test_phev_service_statusAsJson_has_status_object()
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

    phevServiceCtx_t * ctx = phev_service_init(in,out);
    char * str = phev_service_statusAsJson(ctx);
    
    cJSON * json = cJSON_Parse(str);

    cJSON * status = cJSON_GetObjectItemCaseSensitive(json, "status");

    TEST_ASSERT_NOT_NULL(status);
}
void test_phev_service_statusAsJson_has_battery_object()
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

    phevServiceCtx_t * ctx = phev_service_init(in,out);
    char * str = phev_service_statusAsJson(ctx);
    
    cJSON * json = cJSON_Parse(str);

    cJSON * status = cJSON_GetObjectItemCaseSensitive(json, "status");

    cJSON * battery = cJSON_GetObjectItemCaseSensitive(status, "battery");

    TEST_ASSERT_NOT_NULL(battery);
}
void test_phev_service_statusAsJson_has_no_battery_level()
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

    phevServiceCtx_t * ctx = phev_service_init(in,out);
    char * str = phev_service_statusAsJson(ctx);
    
    cJSON * json = cJSON_Parse(str);

    cJSON * status = cJSON_GetObjectItemCaseSensitive(json, "status");

    cJSON * battery = cJSON_GetObjectItemCaseSensitive(status, "battery");

    cJSON * level = cJSON_GetObjectItemCaseSensitive(battery, "level");

    TEST_ASSERT_NULL(level);
}
void test_phev_service_statusAsJson_has_battery_level_correct()
{
    const uint8_t data[] = {50};
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

    phevServiceCtx_t * ctx = phev_service_init(in,out);
    phev_model_setRegister(ctx->model,29,data,1);

    char * str = phev_service_statusAsJson(ctx);
    
    cJSON * json = cJSON_Parse(str);

    cJSON * status = cJSON_GetObjectItemCaseSensitive(json, "status");

    cJSON * battery = cJSON_GetObjectItemCaseSensitive(status, "battery");

    cJSON * level = cJSON_GetObjectItemCaseSensitive(battery, "soc");

    TEST_ASSERT_EQUAL(50,level->valueint);
}
void test_phev_service_outputFilter(void)
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

    phevServiceCtx_t * ctx = phev_service_init(in,out);    const uint8_t data[] = {0x6f,0x04,0x00,0x0a,0x00,0x05};
    message_t * message = msg_utils_createMsg(data, sizeof(data));
    bool outbool = phev_service_outputFilter(ctx->pipe, message);

    TEST_ASSERT_TRUE(outbool);
}
void test_phev_service_outputFilter_no_change(void)
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

    phevServiceCtx_t * ctx = phev_service_init(in,out);    
    const uint8_t inData[] = {0x6f,0x04,0x00,0x0a,0x01,0x05};
    message_t * message = msg_utils_createMsg(inData, sizeof(inData));

    const uint8_t data[] = {1};
    
    phev_model_setRegister(ctx->model,10,data,1);

    bool outbool = phev_service_outputFilter(ctx->pipe, message);

    TEST_ASSERT_FALSE(outbool);
}
void test_phev_service_outputFilter_change(void)
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

    phevServiceCtx_t * ctx = phev_service_init(in,out);    const uint8_t inData[] = {0x6f,0x04,0x00,0x0a,0x00,0x05};
    message_t * message = msg_utils_createMsg(inData, sizeof(inData));

    const uint8_t data[] = {1};
    
    phev_model_setRegister(ctx->model,10,data,1);

    bool outbool = phev_service_outputFilter(ctx->pipe, message);

    TEST_ASSERT_TRUE(outbool);
}
void test_phev_service_inputSplitter_not_null(void)
{
    const char * commands = "{ \"requests\" : [{ \"operation\" :  { \"airCon\" : \"on\" } }, {\"operation\" :  { \"airCon\" : \"off\" } } ] }";

    messageBundle_t * messages = phev_service_inputSplitter(NULL, msg_utils_createMsg(commands, strlen(commands)));

    TEST_ASSERT_NOT_NULL(messages);
}
void test_phev_service_inputSplitter_two_messages_num_messages(void)
{
    const char * commands = "{ \"requests\" : [{ \"operation\" :  { \"airCon\" : \"on\" } }, { \"operation\" :  { \"airCon\" : \"off\" } } ] }";

    messageBundle_t * messages = phev_service_inputSplitter(NULL, msg_utils_createMsg(commands, strlen(commands)));

    TEST_ASSERT_NOT_NULL(messages);
    TEST_ASSERT_EQUAL(2,messages->numMessages);
}
void test_phev_service_inputSplitter_two_messages_first(void)
{
    const char * commands = "{ \"requests\" : [{ \"operation\" :  { \"airCon\" : \"on\" } }, { \"operation\" :  { \"airCon\" : \"off\" } } ] }";

    messageBundle_t * messages = phev_service_inputSplitter(NULL, msg_utils_createMsg(commands, strlen(commands)));
    
    TEST_ASSERT_NOT_NULL(messages);
    TEST_ASSERT_EQUAL(2,messages->numMessages);
    
    cJSON * msg = cJSON_Parse(messages->messages[0]->data);
    
    TEST_ASSERT_NOT_NULL(msg);
    
    cJSON * operation = cJSON_GetObjectItemCaseSensitive(msg,"operation");
    
    TEST_ASSERT_NOT_NULL(msg);
    TEST_ASSERT_NOT_NULL(operation);
    }
void test_phev_service_inputSplitter_two_messages_second(void)
{
    const char * commands = "{ \"requests\": [{ \"operation\" :  { \"airCon\" : \"on\" } }, {\"operation\" :  { \"airCon\" : \"off\" } } ] }";

    messageBundle_t * messages = phev_service_inputSplitter(NULL, msg_utils_createMsg(commands, strlen(commands)));

    TEST_ASSERT_NOT_NULL(messages);
    TEST_ASSERT_EQUAL(2,messages->numMessages);

    cJSON * msg = cJSON_Parse(messages->messages[1]->data);

    TEST_ASSERT_NOT_NULL(msg);
    cJSON * operation = cJSON_GetObjectItemCaseSensitive(msg,"operation");
    
    TEST_ASSERT_NOT_NULL(msg);
    TEST_ASSERT_NOT_NULL(operation);
}
void test_phev_service_end_to_end_operations(void)
{
    const char * commands = "{ \"requests\": [{ \"operation\" :  { \"airCon\" : \"on\" } }, { \"operation\" :  { \"headLights\" : \"off\" } } ] }";
    
    const uint8_t expected[] = {0xf6,0x04,0x00,0x04,0x02,0x00,0xf6,0x04,0x00,0x0a,0x02,0x06};

    test_phev_service_global_in_in_message = msg_utils_createMsg(commands, strlen(commands));

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

    phevServiceCtx_t * ctx = phev_service_init(in,out);
    
    phev_pipe_loop(ctx->pipe);
    TEST_ASSERT_NOT_NULL(test_phev_service_global_out_out_message);
    TEST_ASSERT_EQUAL_MEMORY(expected, test_phev_service_global_out_out_message->data,sizeof(expected));
    
}
void test_phev_service_end_to_end_updated_register(void)
{
    test_phev_service_global_in_in_message = NULL;
    test_phev_service_global_out_in_message = NULL;
    
    const uint8_t message[] = {0x6f,0x04,0x00,0x04,0x02,0x79};

    test_phev_service_global_in_out_message = msg_utils_createMsg(message, sizeof(message));

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

    phevServiceCtx_t * ctx = phev_service_init(in,out);

    phev_service_loop(ctx);

    TEST_ASSERT_NOT_NULL(test_phev_service_global_out_in_message);
    
    cJSON * json = cJSON_Parse(test_phev_service_global_out_in_message->data);

    TEST_ASSERT_NOT_NULL_MESSAGE(json,"Invalid json");

    cJSON * responses = cJSON_GetObjectItemCaseSensitive(json,"responses");

    cJSON * item = NULL;
    TEST_ASSERT_NOT_NULL_MESSAGE(responses,"Responses");
    int i=0;
    cJSON_ArrayForEach(item, responses)
    {
        TEST_ASSERT_NOT_NULL_MESSAGE(item,"No array items");
        cJSON * updatedRegister = cJSON_GetObjectItemCaseSensitive(item,"updatedRegister");
        TEST_ASSERT_NOT_NULL(updatedRegister);
        i++;
    }
 
    TEST_ASSERT_EQUAL(1,i);
}
void test_phev_service_end_to_end_multiple_updated_registers(void)
{
    test_phev_service_global_in_in_message = NULL;
    test_phev_service_global_out_in_message = NULL;
    
    const uint8_t message[] = {0x6f,0x04,0x00,0x04,0x02,0x79,0x6f,0x04,0x00,0x05,0x01,0x79};

    test_phev_service_global_in_out_message = msg_utils_createMsg(message, sizeof(message));

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

    phevServiceCtx_t * ctx = phev_service_init(in,out);

    phev_service_loop(ctx);

    TEST_ASSERT_NOT_NULL(test_phev_service_global_out_in_message);
    
    cJSON * json = cJSON_Parse(test_phev_service_global_out_in_message->data);

    TEST_ASSERT_NOT_NULL_MESSAGE(json,"json");

    cJSON * responses = cJSON_GetObjectItemCaseSensitive(json,"responses");

    TEST_ASSERT_NOT_NULL_MESSAGE(responses,"Checking responses");

    int i = 0;
    
    cJSON * item = NULL;

    cJSON_ArrayForEach(item, responses)
    {
        TEST_ASSERT_NOT_NULL_MESSAGE(item,"No array items");
        i++;
    }
    TEST_ASSERT_EQUAL(2,i);

}
void test_phev_service_jsonResponseAggregator(void)
{
    const char * msg1 = "{ \"updatedRegister\": {\"register\": 4, \"length\": 1,\"data\": [2] } }";
    const char * msg2 = "{ \"updatedRegister\": {\"register\": 5, \"length\": 2,\"data\": [5,2] } }";

    message_t * message1 = msg_utils_createMsg(msg1, strlen(msg1));
    message_t * message2 = msg_utils_createMsg(msg2, strlen(msg2));
    
    messageBundle_t * bundle = malloc(sizeof(messageBundle_t));

    bundle->numMessages = 2;
    bundle->messages[0] = message1;
    bundle->messages[1] = message2;

    message_t * out = phev_service_jsonResponseAggregator(NULL,bundle);

    TEST_ASSERT_NOT_NULL(out);

    cJSON * json = cJSON_Parse(out->data);

    TEST_ASSERT_NOT_NULL(json);
    
    cJSON * responses = cJSON_GetObjectItemCaseSensitive(json,"responses");

    TEST_ASSERT_NOT_NULL(responses);

    cJSON * item = NULL;
    int i = 0;
    cJSON_ArrayForEach(item, responses)
    {
        TEST_ASSERT_NOT_NULL(item);
        TEST_ASSERT_EQUAL_STRING("updatedRegister",item->child->string);
        i++;
    }
    TEST_ASSERT_EQUAL(2,i);
    
}
void test_phev_service_init_settings(void)
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

    phevServiceCtx_t * ctx = phev_service_init(in,out);

    TEST_ASSERT_NULL(ctx->pipe->pipe->in_chain->aggregator);
    TEST_ASSERT_NOT_NULL(ctx->pipe->pipe->out_chain->aggregator);

}
static int test_phev_service_complete_callback_called = 0;

void test_phev_service_complete_callback(phevRegisterCtx_t * ctx)
{
    test_phev_service_complete_callback_called ++;
}
void test_phev_service_register(void)
{
    test_phev_service_complete_callback_called = 0;

    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_service_inHandlerIn,
        .outgoingHandler = test_phev_service_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_service_inHandlerOut,
        .outgoingHandler = test_phev_service_outHandlerOut,
    };
    const char mac[] = {0x11,0x22,0x33,0x44,0x55,0x66};

    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phevServiceCtx_t * ctx = phev_service_initForRegistration(in,out);

    phevRegisterCtx_t * regCtx = phev_service_register(mac, ctx, test_phev_service_complete_callback);

    TEST_ASSERT_NOT_NULL(regCtx);
} 
void test_phev_service_register_complete_called(void)
{
    test_phev_service_complete_callback_called = 0;
    test_phev_service_global_in_in_message = NULL;
    test_phev_service_global_out_in_message = NULL;

    const uint8_t message[] = {0x6f,0x04,0x01,0x10,0x00,0x84};

    test_phev_service_global_in_out_message = msg_utils_createMsg(message, sizeof(message));

    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_service_inHandlerIn,
        .outgoingHandler = test_phev_service_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_service_inHandlerOut,
        .outgoingHandler = test_phev_service_outHandlerOut,
    };
    const char mac[] = {0x11,0x22,0x33,0x44,0x55,0x66};

    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phevServiceCtx_t * ctx = phev_service_initForRegistration(in,out);

    phevRegisterCtx_t * regCtx = phev_service_register(mac, ctx, test_phev_service_complete_callback);

    regCtx->startAck = true;
    regCtx->aaAck = true;
    regCtx->registrationRequest = true;
    regCtx->ecu = true;
    regCtx->remoteSecurity = true;
    regCtx->vin = strdup("1234");

    phev_service_loop(ctx);

    TEST_ASSERT_NOT_NULL(regCtx);
    TEST_ASSERT_EQUAL(1, test_phev_service_complete_callback_called);
}
void test_phev_service_complete_resets_transfomers_callback(phevRegisterCtx_t * ctx)
{
    test_phev_service_complete_callback_called ++;
}

void test_phev_service_register_complete_resets_transformers(void)
{
    test_phev_service_complete_callback_called = 0;
    test_phev_service_global_in_in_message = NULL;
    test_phev_service_global_out_in_message = NULL;

    const uint8_t message[] = {0x6f,0x04,0x01,0x10,0x00,0x84};

    test_phev_service_global_in_out_message = msg_utils_createMsg(message, sizeof(message));

    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_service_inHandlerIn,
        .outgoingHandler = test_phev_service_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_service_inHandlerOut,
        .outgoingHandler = test_phev_service_outHandlerOut,
    };
    const char mac[] = {0x11,0x22,0x33,0x44,0x55,0x66};

    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phevServiceCtx_t * ctx = phev_service_initForRegistration(in,out);

    phevRegisterCtx_t * regCtx = phev_service_register(mac, ctx, test_phev_service_complete_callback);

    regCtx->startAck = true;
    regCtx->aaAck = true;
    regCtx->registrationRequest = true;
    regCtx->ecu = true;
    regCtx->remoteSecurity = true;
    regCtx->vin = strdup("1234");

    phev_service_loop(ctx);

    ctx = phev_service_resetPipeAfterRegistration(ctx);

    TEST_ASSERT_NOT_NULL(ctx);
    TEST_ASSERT_EQUAL(1, test_phev_service_complete_callback_called);
    TEST_ASSERT_NOT_NULL(ctx->pipe->pipe->in_chain);
    TEST_ASSERT_EQUAL(phev_service_jsonInputTransformer,ctx->pipe->pipe->in_chain->inputTransformer);
    TEST_ASSERT_EQUAL(phev_service_jsonOutputTransformer, ctx->pipe->pipe->out_chain->outputTransformer);
}
void test_phev_service_create(void)
{
    test_phev_service_complete_callback_called = 0;
    test_phev_service_global_in_in_message = NULL;
    test_phev_service_global_out_in_message = NULL;

    const uint8_t message[] = {0x6f,0x04,0x01,0x10,0x00,0x84};

    test_phev_service_global_in_out_message = msg_utils_createMsg(message, sizeof(message));

    messagingSettings_t inSettings = {
        .incomingHandler = test_phev_service_inHandlerIn,
        .outgoingHandler = test_phev_service_outHandlerIn,
    };
    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_service_inHandlerOut,
        .outgoingHandler = test_phev_service_outHandlerOut,
    };
    uint8_t mac[] = {0x11,0x22,0x33,0x44,0x55,0x66};

    messagingClient_t * in = msg_core_createMessagingClient(inSettings);
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phevServiceSettings_t settings = {
        .in = in,
        .out = out,
        .mac = mac, 
        .registerDevice = false,
        .eventHandler = NULL,
        .errorHandler = NULL,
        .yieldHandler = NULL,    
    };
 
    phevServiceCtx_t * ctx = phev_service_create(settings);

    TEST_ASSERT_NOT_NULL(ctx);
}

void test_phev_service_getRegister(void)
{
    const uint8_t expectedData[] = {1,2,3,4,5,6};
    uint8_t mac[] = {0x11,0x22,0x33,0x44,0x55,0x66};

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

    phevServiceSettings_t settings = {
        .in = in,
        .out = out,
        .mac = mac, 
        .registerDevice = false,
        .eventHandler = NULL,
        .errorHandler = NULL,
        .yieldHandler = NULL,    
    };
 
    phevServiceCtx_t * ctx = phev_service_create(settings);

    ctx->model->registers[1] = malloc(sizeof(phevRegister_t) + sizeof(expectedData));
    ctx->model->registers[1]->length = sizeof(expectedData);
    memcpy(ctx->model->registers[1]->data,expectedData,sizeof(expectedData));

    TEST_ASSERT_NOT_NULL(ctx);

    phevRegister_t * reg = phev_service_getRegister(ctx, 1);

    TEST_ASSERT_NOT_NULL(reg);

    TEST_ASSERT_EQUAL_MEMORY(expectedData, reg->data, sizeof(expectedData));
}
void test_phev_service_setRegister(void)
{
    const uint8_t expectedData[] = {1,2,3,4,5,6};
    uint8_t mac[] = {0x11,0x22,0x33,0x44,0x55,0x66};

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

    phevServiceSettings_t settings = {
        .in = in,
        .out = out,
        .mac = mac, 
        .registerDevice = false,
        .eventHandler = NULL,
        .errorHandler = NULL,
        .yieldHandler = NULL,    
    };
 
    phevServiceCtx_t * ctx = phev_service_create(settings);

    TEST_ASSERT_NOT_NULL(ctx);

    phev_service_setRegister(ctx,2,expectedData,sizeof(expectedData));

    phevRegister_t * reg = phev_service_getRegister(ctx, 2);

    TEST_ASSERT_NOT_NULL(reg);

    TEST_ASSERT_EQUAL_MEMORY(expectedData, ctx->model->registers[2]->data, sizeof(expectedData));
    
}