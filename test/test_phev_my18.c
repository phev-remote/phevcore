#include "unity.h"
#include "phev.h"
#include "cjson/cJSON.h"
const static uint8_t car_5e[]                   = {0x5e,0x0c,0x00,0x01,0x9c,0x8b,0x69,0x1d,0x0c,0xe9,0xad,0xa5,0x01,0x60};
const static uint8_t car_5e_response[]          = {0xe5,0x04,0x01,0x01,0x00,0xeb};
const static uint8_t car_f6[]                   = {0x27,0x40,0x48,0x52,0x48,0x48,0x4a,0x48,0x48,0xdb};
const static uint8_t car_f6_response[]          = {0xbe,0x4c,0x49,0x52,0x48,0x5d};


static message_t * test_phev_my18_global_in_out_message = NULL;
// 5e0c00019c8b691d0ce9ada50160
// e504010100eb

// car 2740485248484a4848db
// app bb4c4808487f

message_t * test_phev_my18_inHandlerOut(messagingClient_t *client) 
{
    message_t * message = NULL;
    if(test_phev_my18_global_in_out_message) {
        message = msg_utils_copyMsg(test_phev_my18_global_in_out_message);
        test_phev_my18_global_in_out_message = NULL;
    }
    
    return message;
}
static int test_phev_my18_5e_num = 0;
void test_phev_my18_outHandlerOut(messagingClient_t *client, message_t *message) 
{
    TEST_ASSERT_NOT_NULL(message);
    
    switch(test_phev_my18_5e_num)
    {
        case 1: {
            TEST_ASSERT_EQUAL_MEMORY(car_5e_response,message->data,sizeof(car_5e_response));
            test_phev_my18_global_in_out_message = msg_utils_createMsg(car_f6,sizeof(car_f6));
            break;
        }
        case 2: {
            TEST_ASSERT_EQUAL_MEMORY(car_f6_response,message->data,sizeof(car_f6_response));
            break;
        }
    }
    if(test_phev_my18_5e_num > 5) 
    {
        TEST_FAIL_MESSAGE("5E Message not received");
    }
    test_phev_my18_5e_num++;
    
}
void test_phev_my18_5e(void)
{
    uint8_t mac[] = {0x24,0x0d,0xc2,0xc2,0x91,0x85};

    messagingSettings_t outSettings = {
        .incomingHandler = test_phev_my18_inHandlerOut,
        .outgoingHandler = test_phev_my18_outHandlerOut,
    };
    
    messagingClient_t * out = msg_core_createMessagingClient(outSettings);

    phevSettings_t settings = {
        .host = "localhost",
        .mac = mac,
        .out = out,
    };


    phevCtx_t * handle = phev_init(settings);
    
    TEST_ASSERT_NOT_NULL(handle);

    test_phev_my18_global_in_out_message = msg_utils_createMsg(car_5e, sizeof(car_5e));

    phev_start(handle);
    
}
